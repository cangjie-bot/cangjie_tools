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
            if (!fileExists(outLogFile)) {
                return true
            }
            try {
                let logStr: String = unsafe { String.fromUtf8Unchecked(File.readFrom(logFile)) }
                if (toStrerr) {
                    eprint(logStr)
                } else {
                    print(logStr)
                }
            } catch (_: Exception){ 
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
    var buildConfig = BuildConfig()
    var taskMap = TaskMap()
    let buildMutex = Mutex()
    let successPkgs = HashMap<String, Bool>()

    init(buildConfig: BuildConfig) {
        this.buildConfig = buildConfig
    }

    func checkDepFuture(dep: TaskIdentifier, task: CompileTask, futTasks: HashMap<TaskIdentifier, Future<Bool>>,
        sem: Semaphore): Bool {
        var fut: Future<Bool>
        synchronized(this.buildMutex) {
            if (!futTasks.contains(dep)) {
                return true
            }
            fut = futTasks[dep]
        }
        return fut.get()
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

    func build(): Bool {
        let logger = BoundedFileLogger(this.taskMap.size)
        logger.start()

        let sem = Semaphore(maxParallelSize)
        let futTasks = HashMap<TaskIdentifier, Future<Bool>>()
        // Guaranteed valid order follows 'buildConfig.packageList.all'
        for (taskId in this.buildConfig.packageList.allIds) {
            if (!this.taskMap.contains(taskId)) {
                continue
            }

            sem.acquire()
            let task = this.taskMap[taskId]
            let future = spawn { =>
                if (this.buildConfig.globalConfig.isProjectCombine && taskId.fullName == buildConfig.globalConfig.rootName) {
                    for ((k,_) in this.taskMap where k != taskId) {
                        if (!this.checkDepFuture(k, task, futTasks, sem)) {
                            sem.release()
                            logger.release()
                            return false
                        }
                    }
                } else {
                    for (dep in task.requireTasks) {
                        if (!this.checkDepFuture(dep, task, futTasks, sem)) {
                            sem.release()
                            logger.release()
                            return false
                        }
                    }

                    if (let Some(superPkgCfg) <- task.superPkgCfg) {
                        for (dep in superPkgCfg.subPkgSet) {
                            let depId = TaskIdentifier(dep, task.target, None)
                            if (!this.checkDepFuture(depId, task, futTasks, sem)) {
                                sem.release()
                                logger.release()
                                return false
                            }
                        }
                    }
                }

                if (!deleteLog(task)) {
                    sem.release()
                    logger.release()
                    return false
                }

                let cjcCall = constructCjcInvocation(task, this.buildConfig)
                let envBuilder = EnvironmentBuilder()
                envBuilder.prepend(LD_PATH, this.buildConfig.globalConfig.ldPath)
                let flag = runTask(task, cjcCall, envBuilder) 

                sem.release()
                logger.log(task.outLogFile, task.errLogFile)
                return flag
            }

            synchronized(this.buildMutex) {
                futTasks.add(taskId, future)
            }
        }

        var isBuilt: Bool = true
        for ((id, task) in futTasks) {
            if (!task.get()) {
                isBuilt = false
                successPkgs.add(id.fullName, false)
            } else if (!successPkgs.contains(id.fullName)) {
                successPkgs.add(id.fullName, true)
            }
        }

        if (!logger.unsafeWait()) {
            isBuilt = false
        }

        return isBuilt
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
            // TODO: Make it better
            let verbose: String = if (compileTask.isMultiplatform) {
                "Compiling package `${compileTask.fullName}`: ${commandStr}\n"
            } else { 
                "Compiling `${compileTask.sourceSetDir}` part of package `${compileTask.fullName}`: ${commandStr}\n"
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
                    errFile.write("Error: failed to compile package `${compileTask.fullName}`, return code is ${returnCode}\n".toArray())
                    // TODO: improve error message
                    // if (compileTask.isMultiplatform) {
                    //     errFile.write("${SourceLocationsPrinter(compileTask.packagePath, compileTask.allEnabledFeatures, project: compileTask.fullName)}".toArray())
                    // }
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

// Returns topologically ordered set of compiler invocations // TODO: Make better comment
func createCompileTasks(r: ResolveItem, buildConfig: BuildConfig): ?ArrayList<CompileTask> {

    let fullName = r.fullName
    let rootPkgName = r.rootPkgName

    let allEnabledFeatures = r.featureDeducer
        .addFeature(r.features)
        .collect()
    let featureMapping = r.featureDeducer.cleanFeatures() // leaving only mapping to apply them to source set features

    let customizedOption = ArrayList<String>()
    if (buildConfig.customizedOption.size != 0) {
        for (k in buildConfig.customizedOption) {
            if (let Some(option) <- r.customizedOption.get(k)) {
                customizedOption.add(option)
            }
        }
    }

    let requiredForTests = buildConfig.requiredForTests
    let exportForTests = requiredForTests && r.hasTestFiles

    let hasSubPkgs = buildConfig.hasSubPkgs.contains(fullName)
    let isMacro = buildConfig.packageList.macros.contains(fullName)
    let superPkgCfg = r.superPkgCfg
    let isMultiplatform = r.packagePath.isMultiplatform

    let isAnalysisCompilePerformance = !isMultiplatform && buildConfig.globalConfig.isAnalysisCompilePerformance

    let isDebug = buildConfig.isDebug
    let isCov = buildConfig.isCov
    let mockSupported = buildConfig.mockSupported

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

        func createForSingleSourceSet(sourceSetEntry: CJMPPackageInfo, prevStageChir: ?String, prevStageCjo: ?String, prevStageSourceSetDir: ?String): (?CompileTask, ?String, ?String, ?String) {
            let packagePath = sourceSetEntry.srcDir.toString()
            let product = sourceSetEntry.product
            let sourceSetDir = sourceSetEntry.outputSuffix.toString()
            let sourceSetFeatures = sourceSetEntry.features

            let logPath = Path(targetDir).join(".build-logs").join(swapOrgName(rootPkgName)).join(sourceSetDir).toString()
            let outLogFile = Path(logPath).join("${swapOrgName(fullName)}.outlog").toString()
            let errLogFile = Path(logPath).join("${swapOrgName(fullName)}.errlog").toString()

            let supressed = ArrayList<String>()

            let outputType = if (!product) {
                OutputType.Chir
            } else if (r.outputType == Exe && requiredForTests) {
                // to be included in tests as a dependency, the package should be compiled as a library anyway
                // unused main function related warnings should also be suppressed in this scenario
                supressed.add("-Woff=unused-main")
                OutputType.Static
            } else { r.outputType }

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

            if (!createDirectory(targetPath) || !createDirectory(logPath)) {
                return (None, None, None, None)
            }

            let requireTasks = r.requires.iterator().map { it => TaskIdentifier(it, target, None) } |> collectHashSet
            if (let Some(psssd) <- prevStageSourceSetDir) {
                requireTasks.add(TaskIdentifier(fullName, target, psssd))
            }

            let thisStageChir: ?String = if (product) { None } else {
                Path(targetPath).join("${fullName}.chir").toString()
            }
            let thisStageCjo: ?String = if (product) { None } else {
                Path(targetPath).join("${fullName}.cjo").toString()
            }
            let thisStageSourceSetDir: ?String = if (product) {
                None
            } else { sourceSetDir }


            let task = CompileTask(
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
                prevStageChir: prevStageChir,
                prevStageCjo: prevStageCjo,
                isMultiplatform: isMultiplatform,
                supressed: supressed,
                outLogFile: outLogFile,
                errLogFile: errLogFile
            )

            return (task, thisStageChir, thisStageCjo, thisStageSourceSetDir)
        }

        let sourceSetUnwrap = ArrayList<CompileTask>()
        var prevStageChir: ?String = None
        var prevStageCjo: ?String = None
        var prevStageSourceSetDir: ?String = None
        for (sourceSetEntry in r.packagePath._sources) {
            let (task, thisStageChir, thisStageCjo, thisStageSourceSetDir) = createForSingleSourceSet(sourceSetEntry, prevStageChir, prevStageCjo, prevStageSourceSetDir)
            sourceSetUnwrap.add(task ?? return None)
            prevStageChir = thisStageChir
            prevStageCjo = thisStageCjo
            prevStageSourceSetDir = thisStageSourceSetDir
        }
        return sourceSetUnwrap
    }

    let result = ArrayList<CompileTask>()

    assertion { r.targetPlatform || r.nativePlatform }
    if (r.targetPlatform) {
        result.add(all: createForTarget(isNativeForCross: false) ?? return None)
    }
    if (r.nativePlatform) {
        result.add(all: createForTarget(isNativeForCross: true) ?? return None)
    }

    return result
}

func parallelBuild(res: ModuleResolve, buildConfig: BuildConfig): Bool {
    let parallel = ParallelBuilder(buildConfig)

    res.forEachPkg { r: ResolveItem =>
        if (buildConfig.isIncremental && !buildConfig.rebuildList.contains(r.fullName)) {
            return true
        }
        if (!r.hasProdFiles) {
            buildConfig.rebuildList.remove(r.fullName)
            return true
        }
        let compileTasks = createCompileTasks(r, buildConfig) ?? return false
        for (config in compileTasks) {
            parallel.taskMap.add(TaskIdentifier.fromTaskData(config), config)
        }
        return true
    }

    if (!parallel.taskMap.isEmpty()) {
        buildConfig.isRebuild = true
    }

    let parallelResult: Bool = parallel.build()

    for (r in res.resolves) {
        if (let Some(true) <- parallel.successPkgs.get(r.fullName)) {
            r.buildFlag = true
        }
    }

    return parallelResult
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
