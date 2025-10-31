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
import cjpm.config.*

class ParallelBuilder {
    var buildConfig = BuildConfig()
    var taskMap = HashMap<String, PackageTasks>()
    let buildMutex = Mutex()
    var successPkgs = ArrayList<String>()
    let queue = LinkedBlockingQueue<(Option<String>, Option<String>, Bool)>()

    init(buildConfig: BuildConfig) {
        this.buildConfig = buildConfig
    }

    func checkDepFuture(dep: String, task: ResolveConfig, futTasks: HashMap<String, Future<Bool>>,
        sem: Semaphore): Bool {
        var fut: Future<Bool>
        synchronized(this.buildMutex) {
            if (!futTasks.contains(dep)) {
                return true
            }
            fut = futTasks[dep] // TODO: Array
        }
        if (!fut.get()) {
            sem.release()
            this.queue.add((task.outLogFile, task.errLogFile, false))
            return false
        }
        return true
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
        let sem = Semaphore(maxParallelSize)
        let futTasks = HashMap<String, Future<Bool>>() // TODO: Array

        let logFuture = spawn {
            =>
            var count: Int64 = taskMapSize(this.taskMap)
            var logFlag: Bool = true
            while (count > 0) {
                let (outLogFile, errLogFile, isNewLog) = this.queue.remove()
                count--
                if (!isNewLog) {
                    continue
                }
                if (!printAllLogs(outLogFile, errLogFile)) {
                    logFlag = false
                }
            }
            return logFlag
        }

        // Guaranteed valid order follows 'buildConfig.packageList.all'
        for (pkgName in this.buildConfig.packageList.all) {
            if (!this.taskMap.contains(pkgName)) {
                continue
            }

            this.taskMap[pkgName].forEach { task =>
                let future = spawn { =>
                    sem.acquire()
                    if (this.buildConfig.globalConfig.isProjectCombine && pkgName == buildConfig.globalConfig.rootName) {
                        for ((k,_) in this.taskMap) {
                            if (k != pkgName && !this.checkDepFuture(k, task, futTasks, sem)) {
                                return false
                            }
                        }
                    } else {
                        for (dep in task.requirePkgs) {
                            if (!this.checkDepFuture(dep, task, futTasks, sem)) {
                                return false
                            }
                        }

                        if (let Some(superPkgCfg) <- task.superPkgCfg) {
                            for (dep in superPkgCfg.subPkgSet) {
                                if (!this.checkDepFuture(dep, task, futTasks, sem)) {
                                    return false
                                }
                            }
                        }
                    }

                    if (task.checkDepCjo) {
                        var nochange: Bool = true
                        for (depPkg in task.requirePkgs) {
                            if (!checkNoChange(depPkg)) {
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
                            sem.release()
                            this.queue.add((None, None, false))
                            return true
                        }
                    }

                    if (!deleteLog(task)) {
                        sem.release()
                        this.queue.add((task.outLogFile, task.errLogFile, false))
                        return false
                    }

                    var flag: Bool = true
                    if (!runTask(task, this.buildConfig.globalConfig.ldPath)) {
                        flag = false
                    }
                    sem.release()
                    this.queue.add((task.outLogFile, task.errLogFile, true))
                    return flag
                }

                synchronized(this.buildMutex) {
                    futTasks.add(pkgName, future) // TODO: Array
                }
            }

        }

        var isBuilt: Bool = true
        for ((pkgName, task) in futTasks) {
            if (!task.get()) {
                isBuilt = false
            } else {
                successPkgs.add(pkgName)
            }
        }

        if (!logFuture.get()) {
            isBuilt = false
        }

        return isBuilt
    }

    func deleteLog(task: ResolveConfig): Bool {
        var delLog: Bool = true
        if (!deleteFile(task.outLogFile)) {
            delLog = false
        }
        if (!deleteFile(task.errLogFile)) {
            delLog = false
        }
        return delLog
    }

    func runTask(resolveConfig: ResolveConfig, ldPath: HashSet<String>): Bool {
        let envBuilder = EnvironmentBuilder()
        envBuilder.prepend(LD_PATH, ldPath)

        let cjcCalls = ArrayList<ArrayList<String>>()
        if (resolveConfig.isMultiplatform) {
            cjcCalls.add(all: getMultiplatformPipeline(resolveConfig, this.buildConfig))
        } else {
            let args = getCommandArgs(resolveConfig, this.buildConfig, Default, resolveConfig.features)
            args.add(all: getTypeCommand(resolveConfig, this.buildConfig))
            cjcCalls.add(args)
        }

        var execCmdFlag: Bool = true
        for (args in cjcCalls) {
            if (!execCmdFlag) {
                return false
            }
            execCmdFlag = false
            CUR_PARALLEL_SIZE.fetchAdd(1)
            if (!futJudge(args)) {
                calculateParallel(CUR_PARALLEL_SIZE.load(), args)
            }

            let env = getVariables()
            let commandStr = getCmdStr(envBuilder.asCliStrings(env), COMPILE_TOOL, args)
            if (this.buildConfig.isVerbose) {
                let verbose: String = "Compiling package `${resolveConfig.fullName}`: ${commandStr}\n"
                if (!createAndWriteFile(resolveConfig.outLogFile, verbose, mode: Append)) {
                    return false
                }
            }

            if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
                addStartTime(resolveConfig.fullName, commandStr)
            }
            var outFile: File
            var errFile: File
            var outFilePath: String = String()
            var errFilePath: String = String()
            try {
                outFilePath = resolveConfig.outLogFile
                errFilePath = resolveConfig.errLogFile
                outFile = File(resolveConfig.outLogFile, OpenMode.Append)
                errFile = File(resolveConfig.errLogFile, OpenMode.Append)
            } catch (e: Exception) {
                eprintln(e.message)
                eprintln("Error: create '${outFilePath}' failed")
                return false
            }
            try {
                if (let Some(returnCode) <- execAndToFile(COMPILE_TOOL, args, outFile, errFile, envBuilder: envBuilder, originalEnv: env)) {
                    if (returnCode == 0) {
                        execCmdFlag = true
                        if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
                            moveCjcProfToCjpmDir(resolveConfig, buildConfig.globalConfig.compilePerformanceTargetDir)
                        }
                    } else {
                        execCmdFlag = false
                        errFile.write("Error: failed to compile package `${resolveConfig.fullName}`, return code is ${returnCode}\n".toArray())
                        if (resolveConfig.isMultiplatform) {
                            errFile.write("${SourceLocationsPrinter(resolveConfig.packagePath, resolveConfig.features, project: resolveConfig.fullName)}".toArray())
                        }
                    }
                } else {
                    errFile.write("Error: failed to compile package `${resolveConfig.fullName}` with exception occurred\n".toArray())
                }
            } catch (e: Exception) {
                eprintln("Error: failed to write error log into ${errFilePath}: ${e.message}")
                execCmdFlag = false
            }
            CUR_PARALLEL_SIZE.fetchSub(1)
            outFile.close()
            errFile.close()

            if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
                addEndTime(resolveConfig.fullName)
            }
        }
        return execCmdFlag
    }
}

func moveCjcProfToCjpmDir(resolveConfig: ResolveConfig, profcompilePerformancePath: String): Unit {
    let profSuffixArray: Array<String> = [".mem.prof", ".time.prof", ".info.prof"]
    for (profSuffix in profSuffixArray) {
        let pkgNameProfPath = Path(resolveConfig.targetPath).join(resolveConfig.fullName + profSuffix)
        if (exists(pkgNameProfPath)) {
            let profPath = Path(profcompilePerformancePath).join(resolveConfig.fullName + profSuffix)
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

func fromResolveItem(r: ResolveItem, buildConfig: BuildConfig): ?PackageTasks {
    func createResolveConfig(isNativeForCross!: Bool = false): ?ResolveConfig {
        let targetDir = if (isNativeForCross) { 
            buildConfig.globalConfig.nativeDir 
        } else { buildConfig.globalConfig.targetDir }
        let logPath = Path(targetDir).join(".build-logs").join(r.rootPkgName).toString()
        let outLogFile = Path(logPath).join("${r.fullName}.outlog").toString()
        let errLogFile = Path(logPath).join("${r.fullName}.errlog").toString()
        let targetPath = if (r.outputType == Exe && !buildConfig.requiredForTests) {
                Path(targetDir).join(BIN).toString()
            } else {
                Path(targetDir).join(r.rootPkgName).toString()
            }
        if (!createDirectory(targetPath) || !createDirectory(logPath)) {
            return None
        }

        if (r.isMultiplatform) {
            for (source in r.packagePath._sources) {
                let targetSpecificPath = Path(targetPath).join(source.outputSuffix).join(r.rootPkgName).toString()
                if (!createDirectory(targetSpecificPath)) {
                    return None
                }
            }
        }

        let target = if (buildConfig.isCrossCompile && !isNativeForCross) {
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

        let checkDepCjo = buildConfig.isIncremental && 
            r.outputType == Static &&
            buildConfig.indirectRebuilds.contains(r.fullName)

        var result = ResolveConfig()
        result.targetDir = targetDir
        result.targetPath = targetPath
        result.packagePath = r.packagePath
        result.rootPkgName = r.rootPkgName
        result.fullName = r.fullName
        result.outputType = r.outputType
        result.target = target
        result.compileOption = compileOption
        result.overrideOption = overrideOption
        result.linkOption = linkOption
        result.customizedOption = r.customizedOption
        result.requirePkgs = r.requires
        result.outLogFile = outLogFile
        result.errLogFile = errLogFile
        result.checkDepCjo = checkDepCjo
        result.superPkgCfg = r.superPkgCfg
        result.hasTestFiles = r.hasTestFiles
        result.features = r.featureDeducer
            .addFeature(r.features)
            .collect()
        result.featureDeducer = r.featureDeducer
            .cleanFeatures() // leaving only mapping to apply them to source set features

        return result
    }

    let targetConfig: ?ResolveConfig = if (r.nativePlatform) {
        createResolveConfig(isNativeForCross: true) ?? return None
    } else { None }
    let nativeConfig: ?ResolveConfig = if (r.targetPlatform) {
        createResolveConfig(isNativeForCross: false) ?? return None
    } else { None }
    return PackageTasks(
        targetConfig: targetConfig,
        nativeConfig: nativeConfig
    )
}

func parallelBuild(res: ModuleResolve, buildConfig: BuildConfig): Bool {
    let parallel = ParallelBuilder(buildConfig)
    res.forEachPkg(
        {
            r: ResolveItem =>
            let resolveConfigs = fromResolveItem(r, buildConfig) ?? return false
            parallel.taskMap.add(r.fullName, resolveConfigs)
            return true
        }
    )

    if (!parallel.taskMap.isEmpty()) {
        buildConfig.isRebuild = true
    }

    let parallelResult: Bool = parallel.build()

    for (r in res.resolves) {
        if (parallel.successPkgs.contains(r.fullName)) {
            r.buildFlag = true
        }
    }

    return parallelResult
}

func printAllLogs(outLogFile: Option<String>, errLogFile: Option<String>): Bool {
    var logFlag: Bool = true
    func printLog(logFile: ?String): Unit {
        if (let Some(logFile) <- logFile) {
            if (fileExists(logFile) && !loadLogAndPrint(logFile, false)) {
                logFlag = false
            }
        }
    }
    printLog(outLogFile)
    printLog(errLogFile)
    return logFlag
}

func loadLogAndPrint(file: String, isStderr: Bool): Bool {
    var logStr: String = ""
    unsafe {
        logStr = String.fromUtf8Unchecked(File.readFrom(file))
    }

    if (isStderr) {
        eprint(logStr)
    } else {
        print(logStr)
    }
    return true
}

func calculateParallel(currentParalla: Int64, args: ArrayList<String>): Unit {
    var thisParallelSize = currentParalla
    let totalParallelSize = maxParallelSize

    if (currentParalla <= 2) {
        thisParallelSize = totalParallelSize
    } else {
        thisParallelSize = 2 * totalParallelSize / currentParalla
    }
    args.add("-j${thisParallelSize}", at: 0)
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
