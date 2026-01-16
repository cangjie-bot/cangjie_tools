// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.collection.concurrent.*
import std.sync.*
import std.fs.*
import std.regex.*
import std.time.*
import std.env.getVariables
import std.deriving.*

import cjpm.config.*
import cjpm.util.*

type DownStreamTasks = HashMap<TaskIdentifier, HashSet<TaskIdentifier>>
type Task2SyncCounter = HashMap<TaskIdentifier, SyncCounter>

enum Notification {
    | CjoReady
    | ChirReady
    | ObjReady
    | ExeReady
    | DylibReady
    | StaticlibReady
    | Err
}

let outputType2Notification = HashMap<String, Notification>(
    [
        ("static", StaticlibReady),
        ("dynamic", DylibReady),
        ("executable", ExeReady),
        ("chir", ChirReady),
        ("obj", ObjReady)
    ]
)

type TaskMap = HashMap<TaskIdentifier, CompileTask>

private enum MessageKind {
    | Release
    | Print(String, String)
}

// A very simple logger which can write at most `messageBound` messages read from files
public class BoundedFileLogger {
    private let messageQueue = LinkedBlockingQueue<MessageKind>()
    private var logFuture: ?Future<Bool> = None

    public BoundedFileLogger(
        private let messageBound: Int64
    ) { }

    public func release(): Unit {
        this.messageQueue.add(Release)
    }

    public func log(outLogFile: String, errLogFile: String): Unit {
        this.messageQueue.add(Print(outLogFile, errLogFile))
    }

    public func start(): Unit {
        if (logFuture.isNone()) {
            this.logFuture = spawn { =>
                var count = this.messageBound
                var success: Bool = true
                while (count > 0) {
                    match (this.messageQueue.remove()) {
                        case Release => count--
                        case Print(outLogFile, errLogFile) => 
                            count--
                            success = printLogs(outLogFile, errLogFile) && success
                    }
                }
                return success
            }
        }
    }

    // If number of `log` and `release` calls is below the `messageBound`, deadlock occurs
    func unsafeWait(): Bool {
        if (let Some(logFuture) <- this.logFuture) {
            logFuture.get()
        } else { true }
    }


    private static func printLogs(outLogFile: String, errLogFile: String): Bool {
        var logFlag: Bool = true
        func printLogFile(logFile: String, toStrerr: Bool): Bool {
            if (!fileExists(logFile)) {
                return true
            }
            try {
                let logStr: String = unsafe { String.fromUtf8Unchecked(File.readFrom(logFile)) }
                if (toStrerr) {
                    eprint(logStr)
                } else {
                    print(logStr)
                }
            } catch (e: Exception) {
                eprintln(e.message)
                eprintln("Error: failed to read '${logFile}'")
                return false
            }
            return true
        }
        logFlag = printLogFile(outLogFile, false) && logFlag
        logFlag = printLogFile(errLogFile, true) && logFlag
        return logFlag
    }
}

class ParallelBuilder {
    let buildConfig: BuildConfig
    let taskMap: TaskMap
    let downstreamTasks: DownStreamTasks
    let scs: Task2SyncCounter
    let orderedTasks: Array<TaskIdentifier>
    let successPkgs = HashMap<String, Bool>()
    let notifyMutex = Mutex()
    let notified = DownStreamTasks()

    init(buildConfig: BuildConfig, taskMap: TaskMap, downstreamTasks: DownStreamTasks,
        scs: Task2SyncCounter, orderedTasks: Array<TaskIdentifier>) {
        this.buildConfig = buildConfig
        this.taskMap = taskMap
        this.downstreamTasks = downstreamTasks
        this.scs = scs
        this.orderedTasks = orderedTasks
        orderedTasks.forEach { task => notified.add(task, HashSet<TaskIdentifier>()) }
    }

    func checkNoChange(depPkg: String): Bool {
        if (buildConfig.rebuildMacros.contains(depPkg)) {
            return false
        }
        let rootPkgName: String = getRootPkgName(depPkg)
        let cjoFile = Path(this.buildConfig.globalConfig.targetDir).join(rootPkgName).join("${depPkg}.cjo").toString()
        if (!fileExists(cjoFile) || !buildConfig.cjoTimeMap.contains(depPkg) || buildConfig.cjoTimeMap[depPkg] !=
            FileInfo(cjoFile).lastModificationTime.toUnixTimeStamp().toSeconds()) {
            return false
        }
        return true
    }

    private func shouldNotify(downstreamTask: TaskIdentifier, task: TaskIdentifier, notification: Notification): Bool {
        synchronized (this.notifyMutex) {
	        if (!notified.get(task).getOrThrow().add(downstreamTask)) {
	            return false
	        }
	    }
        match (notification) {
            case CjoReady =>
                if (taskMap.get(task)?.isMacro ?? true) {
                    false
                } else {
                    match (downstreamTask.compilePhase) {
                        case LinkOnly => false
                        case SkipLink => true
                        case Default =>
                            if (let Some(compileTask) <- taskMap.get(downstreamTask)) {
                                !compileTask.isLto &&
                                !compileTask.isMultiplatform &&
                                compileTask.outputType == OutputType.Static
                            } else { false }
                    }
                }
            case _ => true
        }
    }

    private func notify(task: TaskIdentifier, notification: Notification): Unit {
        for (t in downstreamTasks[task]) {
            if (shouldNotify(t, task, notification)) {
                scs[t].dec()
            }
        }
    }

    private func getCjo2TaskAndListenDirs(): (HashMap<String, TaskIdentifier>, HashSet<String>) {
        let cjo2Task = HashMap<String, TaskIdentifier>()
        let listenDirs = HashSet<String>()
        for (taskId in orderedTasks) {
            let task = this.taskMap[taskId]
            listenDirs.add(task.targetPath)
            cjo2Task.add(Path(task.targetPath).join("${task.fullName}.cjo.flag").toString(), taskId)
        }
        (cjo2Task, listenDirs)
    }

    func build(): Bool {
        let logger = BoundedFileLogger(this.taskMap.size)
        logger.start()

        let (cjo2Task, listenDirs) = getCjo2TaskAndListenDirs()
        let watchFSFuture = spawn { => watchFS(cjo2Task, listenDirs, notify)}

        let sem = Semaphore(maxParallelSize)
        let futTasks = HashMap<TaskIdentifier, Future<Bool>>()
        let buildResult = AtomicBool(true)
        // Guaranteed valid order follows 'buildConfig.packageList.all'
        for (taskId in orderedTasks) {
            sem.acquire()
            let task = this.taskMap[taskId]
            let action = { =>
                scs[taskId].waitUntilZero()
                if (!buildResult.load()) {
                    logger.release()
                    return false
                }
                if (task.checkDepCjo) {
                    var nochange: Bool = true
                    for (depPkg in task.requireTasks) {
                        if (!checkNoChange(depPkg.fullName)) {
                            nochange = false
                            break
                        }
                    }
                    let subPkgSet = task.superPkgCfg?.subPkgSet ?? HashSet<String>()
                    for (subPkg in subPkgSet) {
                        if (!checkNoChange(subPkg)) {
                            nochange = false
                            break
                        }
                    }
                    if (nochange) {
                        logger.release()
                        return true
                    }
                }

                if (!deleteLog(task)) {
                    logger.release()
                    return false
                }

                let cjcCall = constructCjcInvocation(task, this.buildConfig)
                let envBuilder = EnvironmentBuilder()
                envBuilder.prepend(LD_PATH, this.buildConfig.globalConfig.ldPath)
                let res = runTask(task, cjcCall, envBuilder) 
                logger.log(task.outLogFile, task.errLogFile)
                res
            }
            
            let future = spawn { =>
                let res = action()
                sem.release()
                if (!res) {
                    buildResult.store(false)
                }
                let notification = if (res) {
                    outputType2Notification.get(task.outputType.toString()).getOrThrow()
                } else { Err }
                notify(taskId, notification)
                res
            }

            futTasks.add(taskId, future)
        }

        for ((id, task) in futTasks) {
            if (!task.get()) {
                successPkgs.add(id.fullName, false)
            } else {
                successPkgs.addIfAbsent(id.fullName, true)
            }
        }

        if (!logger.unsafeWait()) {
            buildResult.store(false)
        }

        //unsafe { stopFSWatcher() }
        watchFSFuture.get()

        buildResult.load()
    }

    func deleteLog(task: CompileTask): Bool {
        var delLog: Bool = true
        if (!deleteFile(task.outLogFile)) {
            delLog = false
        }
        if (!deleteFile(task.errLogFile)) {
            delLog = false
        }
        return delLog
    }

    func runTask(compileTask: CompileTask, args: ArrayList<String>, envBuilder: EnvironmentBuilder): Bool {
        var execCmdFlag: Bool = true
        CUR_PARALLEL_SIZE.fetchAdd(1)
        if (!futJudge(args)) {
            let jobs = calculateParallel(CUR_PARALLEL_SIZE.load())
            args.add("-j${jobs}", at: 0)
        }

        let env = getVariables()
        let commandStr = getCmdStr(envBuilder.asCliStrings(env), COMPILE_TOOL, args)
        if (this.buildConfig.isVerbose) {
            let verbose: String = if (compileTask.isMultiplatform) {
                "Compiling `${compileTask.sourceSetDir}` part of package `${compileTask.fullName}`: ${commandStr}\n"
            } else { 
                "Compiling package `${compileTask.fullName}`: ${commandStr}\n"
            }
            if (!createAndWriteFile(compileTask.outLogFile, verbose, mode: Append)) {
                return false
            }
        }

        if (compileTask.isAnalysisCompilePerformance) {
            addStartTime(compileTask.fullName, commandStr)
        }

        let outFilePath = compileTask.outLogFile
        let errFilePath = compileTask.errLogFile
        let (outFile, errFile) = try {
            (File(outFilePath, OpenMode.Append), File(errFilePath, OpenMode.Append))
        } catch (e: Exception) {
            eprintln(e.message)
            eprintln("Error: create '${outFilePath}' failed")
            return false
        }

        try {
            if (let Some(returnCode) <- execAndToFile(COMPILE_TOOL, args, outFile, errFile, envBuilder: envBuilder, originalEnv: env)) {
                execCmdFlag = (returnCode == 0)
                if (execCmdFlag) {
                    if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
                        moveCjcProfToCjpmDir(compileTask, buildConfig.globalConfig.compilePerformanceTargetDir)
                    }
                } else {
                    let errmesg = if (compileTask.isMultiplatform) {
                        "Error: failed to compile `${compileTask.sourceSetDir}` part of package `${compileTask.fullName}`, return code is ${returnCode}\n"
                    } else {
                        "Error: failed to compile package `${compileTask.fullName}`, return code is ${returnCode}\n"
                    }
                    errFile.write(errmesg.toArray())
                }
            } else {
                errFile.write("Error: failed to compile package `${compileTask.fullName}` with exception occurred\n".toArray())
            }
        } catch (e: Exception) {
            eprintln("Error: failed to write error log into ${errFilePath}: ${e.message}")
            execCmdFlag = false
        }
        CUR_PARALLEL_SIZE.fetchSub(1)
        outFile.close()
        errFile.close()

        if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
            addEndTime(compileTask.fullName)
        }
        return execCmdFlag
    }
}

func moveCjcProfToCjpmDir(compileTask: CompileTask, profcompilePerformancePath: String): Unit {
    let profSuffixArray: Array<String> = [".mem.prof", ".time.prof", ".info.prof"]
    for (profSuffix in profSuffixArray) {
        let pkgNameProfPath = Path(compileTask.targetPath).join(compileTask.fullName + profSuffix)
        if (exists(pkgNameProfPath)) {
            let profPath = Path(profcompilePerformancePath).join(compileTask.fullName + profSuffix)
            copy(pkgNameProfPath, to: profPath, overwrite: true)
            remove(pkgNameProfPath)
        } else {
            eprintln("Error: cannot find ${pkgNameProfPath.toString()} file")
        }
    }
    return
}

func addStartTime(fullName: String, cmdArgs: String): Unit {
    synchronized(CACHE_MUTEX) {
        SHOW_CACHE.add(PkgInfo(fullName, "B", DateTime.now().toUnixTimeStamp().toMilliseconds()))
        COMMAND_CACHE.add(CommandInfo(fullName, cmdArgs))
    }
    return
}

func addEndTime(fullName: String): Unit {
    synchronized(CACHE_MUTEX) {
        SHOW_CACHE.add(PkgInfo(fullName, "E", DateTime.now().toUnixTimeStamp().toMilliseconds()))
    }
    return
}

private struct PrevStage {
    PrevStage(
        let chir!: String,
        let commonCjo!: String,
        let taskId!: TaskIdentifier
    ) { }
}

// Returns an array of compiler calls, which are required to compile a package
func createCompileTasks(r: ResolveItem, buildConfig: BuildConfig): ?ArrayList<CompileTask> {
    // Common to all tasks, but needs to be embedded in CompileTask struct
    let isDebug = buildConfig.isDebug
    let isCov = buildConfig.isCov
    let requiredForTests = buildConfig.requiredForTests
    let mockSupported = buildConfig.mockSupported

    // Common to all tasks, needed to compile the package
    let fullName = r.fullName
    let rootPkgName = r.rootPkgName
    let isMultiplatform = r.packagePath.isMultiplatform
    let customizedOption = constructCustomizedOption(r, buildConfig)
    let hasSubPkgs = buildConfig.hasSubPkgs.contains(fullName)
    let exportForTests = requiredForTests && r.hasTestFiles
    let isMacro = buildConfig.packageList.isMacro(fullName)
    let superPkgCfg = r.superPkgCfg
    let isAnalysisCompilePerformance = !isMultiplatform && buildConfig.globalConfig.isAnalysisCompilePerformance

    let allEnabledFeatures = r.featureDeducer
        .addFeature(r.features)
        .collect()
    let featureMapping = r.featureDeducer.cleanFeatures() // leaving only mapping to apply them to source set features

    // Specific to the `--target` platform
    // `isNativeForCross == true` - package needs to be compiled with `host` target to be used by or is a `macro package` 
    func createForTarget(isNativeForCross!: Bool = false): ?ArrayList<CompileTask> {
        let targetDir = if (isNativeForCross) { buildConfig.globalConfig.nativeDir } else { buildConfig.globalConfig.targetDir }

        let isCrossCompile = buildConfig.isCrossCompile && !isNativeForCross

        let target = if (isCrossCompile) {
            crossCompileTarget
        } else { targetConfigName }

        let compileOption = if (isNativeForCross) {
            r.nativeCompileOption
        } else { r.compileOption }

        let overrideOption = if (isNativeForCross) {
            buildConfig.globalConfig.nativeOverrideOption
        } else { buildConfig.globalConfig.overrideCompileOption }

        let linkOption = if (isNativeForCross) {
            r.nativeLinkOption
        } else { r.linkOption }

        // Multiplatform packages need an arbitrary number of `cjc` calls
        // Non-multiplatform packages consist of single `sourceSetEntry`
        func createForSingleSourceSet(sourceSetEntry: CJMPPackageInfo, prevStage: ?PrevStage):
            (?ArrayList<CompileTask>, ?PrevStage) {
            let packagePath = sourceSetEntry.srcDir.toString()
            let product = sourceSetEntry.product
            let sourceSetDir = sourceSetEntry.outputSuffix.toString()
            let sourceSetFeatures = sourceSetEntry.features

            let supressed = ArrayList<String>()

            let outputType = if (!product) {
                OutputType.Chir
            } else if (r.outputType == Exe && requiredForTests) {
                // to be included in tests as a dependency, the package should be compiled as a library anyway
                // unused main function related warnings should also be suppressed in this scenario
                supressed.add("-Woff=unused-main")
                OutputType.Static
            } else { r.outputType }

            let checkDepCjo = buildConfig.isIncremental && 
                r.outputType == Static &&
                buildConfig.indirectRebuilds.contains(r.fullName)

            let isLto = buildConfig.isLto && (outputType == Static || outputType == Exe)
            let ltoValue = if (isLto) { buildConfig.ltoValue } else { "" }

            let filename = if (outputType == Exe && buildConfig.packageList.exe.size <= 1 && !COMMON_INFO.inWorkspace) {
                buildConfig.exeName
            } else { fullName }

            let targetPath = if (outputType == Exe) {
                Path(targetDir).join(BIN).join(sourceSetDir).toString()
            } else {
                Path(targetDir).join(swapOrgName(rootPkgName)).join(sourceSetDir).toString()
            }

            let logPath = Path(targetDir).join(".build-logs").join(swapOrgName(rootPkgName)).join(sourceSetDir).toString()
            if (!createDirectory(targetPath) || !createDirectory(logPath)) {
                return (None, None)
            }

            let requireTasks = r.requires.iterator().map { name =>
                let isMacro = buildConfig.packageList.isMacro(name)
                let waitTarget = if (isMacro) { targetConfigName } else { target }
                let needLink = !isMultiplatform &&
                               !buildConfig.isLto &&
                               !isMacro &&
                               (buildConfig.packageList.isDyLib(name) || buildConfig.packageList.isExe(name))
                let compilePhase = if (needLink) { SkipLink } else { CompilePhase.Default }
                TaskIdentifier(name, waitTarget, "", compilePhase: compilePhase)
            } |> collectHashSet
            if (isMacro && isCrossCompile) {
                requireTasks.add(TaskIdentifier(fullName, targetConfigName, "", compilePhase: CompilePhase.Default))
            }
            requireTasks.addIfSome(prevStage?.taskId)
            let prevStageArtifacts: ?(String, String) = if (let Some(prevStage) <- prevStage) {
                (prevStage.chir, prevStage.commonCjo)
            } else { None }

            let needLink = !isMultiplatform &&
                           !buildConfig.isLto &&
                           !isMacro &&
                           (outputType == Dynamic || outputType == Exe)

            let compilePhase = if (needLink) { SkipLink } else { CompilePhase.Default }

            let taskId = TaskIdentifier(
                fullName,
                target,
                if (!product) { sourceSetDir } else { "" },
                compilePhase: compilePhase
            )

            let (outLogFile, errLogFile) = getLogFile(logPath, taskId)

            let thisStage: ?PrevStage = if (!product) {
                PrevStage(
                    chir: Path(targetPath).join("${fullName}.chir").toString(),
                    commonCjo: Path(targetPath).join("${fullName}.cjo").toString(),
                    taskId: taskId
                )
            } else { None } 

            let task = CompileTask(
                id: taskId,
                targetDir: targetDir,
                targetPath: targetPath,
                packagePath: packagePath,
                product: product,
                sourceSetDir: sourceSetDir,
                sourceSetFeatures: sourceSetFeatures,
                rootPkgName: rootPkgName,
                fullName: fullName,
                filename: filename,
                outputType: if (needLink) { Obj } else { outputType },
                target: target,
                isCrossCompile: isCrossCompile,
                compileOption: compileOption,
                overrideOption: overrideOption,
                linkOption: linkOption,
                customizedOption: customizedOption,
                superPkgCfg: superPkgCfg,
                requiredForTests: requiredForTests,
                exportForTests: exportForTests,
                requireTasks: requireTasks,
                allEnabledFeatures: allEnabledFeatures,
                featureMapping: featureMapping,
                isAnalysisCompilePerformance: isAnalysisCompilePerformance,
                hasSubPkgs: hasSubPkgs,
                isDebug: isDebug,
                isCov: isCov,
                mockSupported: mockSupported,
                isMacro: isMacro,
                isLto: isLto,
                ltoValue: ltoValue,
                prevStageArtifacts: prevStageArtifacts,
                isMultiplatform: isMultiplatform,
                checkDepCjo: checkDepCjo,
                supressed: supressed,
                outLogFile: outLogFile,
                errLogFile: errLogFile,
                compileTarget: if (needLink) { outputType } else { None }
            )

            let compileTasks = ArrayList<CompileTask>([task])

            if (needLink) {
                let taskId = TaskIdentifier(
                    fullName,
                    target,
                    if (!product) { sourceSetDir } else { "" },
                    compilePhase: LinkOnly
                )
                let prevStageArtifacts = (Path(task.targetPath).join(getOutput(task).getOrThrow()).toString(), "")
                let requireTasks = getDepSet(fullName, buildConfig).iterator().filterMap { name =>
                        if(!buildConfig.packageList.isMacro(name) && !buildConfig.packageList.isTest(name)) {
                            let needLink = buildConfig.packageList.isDyLib(name) || buildConfig.packageList.isExe(name)
                            let compilePhase = if (needLink) { LinkOnly } else { CompilePhase.Default }
                            TaskIdentifier(name, target, "", compilePhase: compilePhase)
                        } else {
                            None
                        }
                    } |> collectHashSet
                requireTasks.add(task.id)
                let (outLogFile, errLogFile) = getLogFile(logPath, taskId)
                let linkTask = CompileTask(
                    id: taskId,
                    targetDir: targetDir,
                    targetPath: targetPath,
                    packagePath: packagePath,
                    product: product,
                    sourceSetDir: sourceSetDir,
                    sourceSetFeatures: sourceSetFeatures,
                    rootPkgName: rootPkgName,
                    fullName: fullName,
                    filename: filename,
                    outputType: outputType,
                    target: target,
                    isCrossCompile: isCrossCompile,
                    compileOption: compileOption,
                    overrideOption: overrideOption,
                    linkOption: linkOption,
                    customizedOption: customizedOption,
                    superPkgCfg: superPkgCfg,
                    requiredForTests: requiredForTests,
                    exportForTests: exportForTests,
                    requireTasks: requireTasks,
                    allEnabledFeatures: allEnabledFeatures,
                    featureMapping: featureMapping,
                    isAnalysisCompilePerformance: isAnalysisCompilePerformance,
                    hasSubPkgs: hasSubPkgs,
                    isDebug: isDebug,
                    isCov: isCov,
                    mockSupported: mockSupported,
                    isMacro: isMacro,
                    isLto: isLto,
                    ltoValue: ltoValue,
                    prevStageArtifacts: prevStageArtifacts,
                    isMultiplatform: isMultiplatform,
                    checkDepCjo: checkDepCjo,
                    supressed: supressed,
                    outLogFile: outLogFile,
                    errLogFile: errLogFile
                )
                compileTasks.add(linkTask)
            }

            (compileTasks, thisStage)
        }

        let sourceSetUnwrap = ArrayList<CompileTask>()
        var prevStage: ?PrevStage = None
        for (sourceSetEntry in r.packagePath._sources) {
            let (tasks, thisStage) = createForSingleSourceSet(sourceSetEntry, prevStage)
            sourceSetUnwrap.add(all: tasks ?? return None)
            prevStage = thisStage
        }
        sourceSetUnwrap
    }

    let result = ArrayList<CompileTask>()

    assertion { r.targetPlatform || r.nativePlatform }
    if (r.nativePlatform) {
        result.add(all: createForTarget(isNativeForCross: true) ?? return None)
    }
    if (r.targetPlatform) {
        result.add(all: createForTarget(isNativeForCross: false) ?? return None)
    }
    return result
}

private func constructCustomizedOption(r: ResolveItem, buildConfig: BuildConfig): ArrayList<String> {
    let customizedOption = ArrayList<String>()
    for (k in buildConfig.customizedOption) {
        if (let Some(option) <- r.customizedOption.get(k)) {
            customizedOption.add(option)
        }
    }
    return customizedOption
}

private func getDownstreamTasksAndSynCounters(buildConfig: BuildConfig, taskMap: TaskMap,
    orderedTasks: ArrayList<TaskIdentifier>): (DownStreamTasks, Task2SyncCounter) {
    let downstreamTasks = DownStreamTasks()
    let scs = Task2SyncCounter()
    for (taskId in orderedTasks where taskMap.contains(taskId)) {
        let depTasks = HashSet<TaskIdentifier>()
        downstreamTasks.addIfAbsent(taskId, HashSet<TaskIdentifier>())
        if (buildConfig.globalConfig.isProjectCombine && taskId.fullName == buildConfig.globalConfig.rootName) {
            for ((k, _) in taskMap where k != taskId && taskMap.contains(k)) {
                depTasks.add(k)
                downstreamTasks.addIfAbsent(k, HashSet<TaskIdentifier>())
                downstreamTasks[k].add(taskId)
            }
        } else {
            let task = taskMap[taskId]
            for (dep in task.requireTasks where taskMap.contains(dep)) {
                depTasks.add(dep)
                downstreamTasks.addIfAbsent(dep, HashSet<TaskIdentifier>())
                downstreamTasks[dep].add(taskId)
            }

            if (let Some(superPkgCfg) <- task.superPkgCfg) {
                for (dep in superPkgCfg.subPkgSet) {
                    let depId = TaskIdentifier(dep, task.target, "")
                    if (!taskMap.contains(depId)) {
                        continue
                    }
                    depTasks.add(depId)
                    downstreamTasks.addIfAbsent(depId, HashSet<TaskIdentifier>())
                    downstreamTasks[depId].add(taskId)
                }
            }
        }
        scs.add(taskId, SyncCounter(depTasks.size))
    }
    (downstreamTasks, scs)
}

func parallelBuild(res: ModuleResolve, buildConfig: BuildConfig): Bool {
    let (taskMap, orderedTasks) = res.resolves.fold((TaskMap(), ArrayList<TaskIdentifier>())) {
        acc, r: ResolveItem =>
        if (buildConfig.isIncremental && !buildConfig.rebuildList.contains(r.fullName)) {
            return acc
        }
        if (!r.hasProdFiles) {
            buildConfig.rebuildList.remove(r.fullName)
            return acc
        }
        let compileTasks = createCompileTasks(r, buildConfig) ?? return acc
        for (task in compileTasks) {
            acc[0].add(task.id, task)
            acc[1].add(task.id)
        }
        acc
    }

    if (!taskMap.isEmpty()) {
        buildConfig.isRebuild = true
    }

    let (downstreamTasks, scs) = getDownstreamTasksAndSynCounters(buildConfig, taskMap, orderedTasks)

    let parallel = ParallelBuilder(buildConfig, taskMap, downstreamTasks, scs, orderedTasks.toArray())

    let parallelResult: Bool = parallel.build()

    for (r in res.resolves) {
        if (let Some(true) <- parallel.successPkgs.get(r.fullName)) {
            r.buildFlag = true
        }
    }

    parallelResult
}

func calculateParallel(currentParalla: Int64): Int64 {
    var thisParallelSize = currentParalla
    let totalParallelSize = maxParallelSize

    if (currentParalla <= 2) {
        thisParallelSize = totalParallelSize
    } else {
        thisParallelSize = 2 * totalParallelSize / currentParalla
    }
    return thisParallelSize
}

func futJudge(args: ArrayList<String>): Bool {
    let tmp = args.toString()
    let fut1: Future<Bool> = spawn {
        let regexParallel = Regex("-j[[:space:]]*(?:[[:digit:]]|=[[:digit:]])")
        regexParallel.matches(tmp)
    }
    let fut2: Future<Bool> = spawn {
        tmp.contains("--jobs") || args.contains("-j")
    }
    return (fut1.get() || fut2.get())
}

func getLogFile(dir: String, taskId: TaskIdentifier): (String, String) {
    let logFileName = "${taskId.fullName}.${taskId.target}.${taskId.sourceSetDir}.${taskId.compilePhase.toString()}"
	let outLogFile = Path(dir).join("${logFileName}.outlog").toString()
	let errLogFile = Path(dir).join("${logFileName}.errlog").toString()
    (outLogFile, errLogFile)
}

var watcherCb: (String) -> Unit = { _ => }

func watchFS(cjo2task: HashMap<String, TaskIdentifier>, listenDirs: HashSet<String>, notify: (TaskIdentifier, Notification) -> Unit): Unit {
    let dirs = StringBuilder()
    for (dir in listenDirs) {
        if (!exists(dir)) {
            Directory.create(dir, recursive: true)
        }
        dirs.append("${dir},")
    }
    watcherCb = { cjo => notify(cjo2task[cjo], CjoReady) }
    // unsafe {
    //     let cs = LibC.mallocCString(dirs.toString())
    //     let res = initFSWatcher(cs, { cjo => watcherCb(cjo.toString()) })
    //     LibC.free(cs)
    //     if (!res) {
    //         return
    //     }
    //     startFSWatcher()
    // }
}

// foreign func initFSWatcher(dirs: CString, cb: CFunc<(CString) -> Unit>): Bool

// foreign func startFSWatcher(): Unit

// foreign func stopFSWatcher(): Unit