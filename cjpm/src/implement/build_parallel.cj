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

var cb: (String) -> Unit = {_=>}

foreign func CMonitor(s: CString, cb: CFunc<(CString) -> Unit>): Int64

func monitor(macroSet: HashSet<String>, cjo2pkg: HashMap<String, String>, listenDirs: HashSet<String>, notify: (String) -> Unit): Unit {
    var str = ""
    for (dir in listenDirs) {
        if(!exists(dir)) {
            Directory.create(dir, recursive: true)
        }
        str += dir + ","
    }
    cb = {
        cjopath =>
        let pkgname = cjo2pkg[cjopath]
        if(!macroSet.contains(pkgname)) {
            notify(pkgname)
        }
    }
    unsafe {
        let cs = LibC.mallocCString(str)
        CMonitor(cs, {dir => cb(dir.toString())})
        LibC.free(cs)
    }
}

class ParallelBuilder {
    var buildConfig = BuildConfig()
    var taskMap = HashMap<String, ResolveConfig>()
    let buildMutex = Mutex()
    var successPkgs = ArrayList<String>()
    let queue = LinkedBlockingQueue<(Option<String>, Option<String>, Option<String>, Option<String>, Bool)>()
    let rDeps = HashMap<String, HashSet<String>>()
    let sems = HashMap<String, SyncCounter>()
    let res = HashMap<String, Bool>()
    let nMutex = Mutex()
    let notified = HashSet<String>()

    init(buildConfig: BuildConfig) {
        this.buildConfig = buildConfig
    }

    func notify(pkgName: String) {
        synchronized (this.nMutex) {
            if(notified.contains(pkgName)) {
                return
            }
            notified.add(pkgName)
        }
        for (pkg in rDeps[pkgName]) {
            println("'"+pkgName +"' notify '"+pkg+"'")
            sems[pkg].dec()
        }
    }

    func notifyPh2(pkgName: String) {
        println("'"+pkgName +"' notify '"+pkgName+".ph2'")
        sems[pkgName + ".ph2"].dec()
    }

    func checkDepFuture(dep: String, task: ResolveConfig, futTasks: HashMap<String, Future<Bool>>,
        sem: Semaphore): Bool {
        var fut: Future<Bool>
        synchronized(this.buildMutex) {
            if (!futTasks.contains(dep)) {
                return true
            }
            fut = futTasks[dep]
        }
        if (!fut.get()) {
            sem.release()
            this.queue.add((task.outLogFile, task.errLogFile, None, None, false))
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
        println("ParallelBuilder build lz")
        let sem = Semaphore(maxParallelSize)
        let futTasks = HashMap<String, Future<Bool>>()

        let logFuture = spawn {
            =>
            var count: Int64 = this.taskMap.size
            var logFlag: Bool = true
            while (count > 0) {
                let (outLogFile, errLogFile, nativeOutLog, nativeErrLog, isNewLog) = this.queue.remove()
                count--
                if (!isNewLog) {
                    continue
                }
                if (!printAllLogs(outLogFile, errLogFile, nativeOutLog, nativeErrLog)) {
                    logFlag = false
                }
            }
            return logFlag
        }

        let cjo2pkg = HashMap<String, String>()
        let listenDirs = HashSet<String>()

        let macroSet = HashSet<String>()
        let macroPkgList = buildConfig.packageList.macros
        for (m in macroPkgList) {
            macroSet.add(m)
        }

        for (pkgName in buildConfig.packageList.all) {
            if (!taskMap.contains(pkgName)) {
                continue
            }
            let task = taskMap[pkgName]
            let outputdir = if (task.nativePlatform) {task.nativePath} else {task.targetPath}
            listenDirs.add(outputdir)
            cjo2pkg.add(outputdir + "/" + pkgName + ".cjo.flag", pkgName)
            if (!rDeps.contains(pkgName)) {
                rDeps.add(pkgName, HashSet<String>())
            }
            if (!rDeps.contains(pkgName + ".ph2")) {
                rDeps.add(pkgName + ".ph2", HashSet<String>())
            }
            var cnt = 0
            for (dep in task.requirePkgs) {
                if (!taskMap.contains(dep)) {
                    continue
                }
                cnt++
                if (!rDeps.contains(dep)) {
                    rDeps.add(dep, HashSet<String>())
                }
                rDeps[dep].add(pkgName)
            }
            sems.add(pkgName, SyncCounter(cnt))
            println("sems "+pkgName+": "+cnt.toString())
            let outputType = task.outputType.toString()
            let needLink = outputType == "dynamic" || outputType == "executable"
            if (needLink) {
                let linkSet = HashSet<String>()
                let linkSort = getDepSet(pkgName, buildConfig.requiresMap, buildConfig.superPkgSubMap, isForMacroRequire: macroPkgList.contains(pkgName))
                for (link in linkSort) {
                    if (!taskMap.contains(link)) {
                        continue
                    }
                    if(!macroSet.contains(link)) { // 不需要 link 宏包
                        if (!rDeps.contains(link + ".ph2")) {
                            rDeps.add(link + ".ph2", HashSet<String>())
                        }
                        rDeps[link + ".ph2"].add(pkgName + ".ph2")
                        linkSet.add(link)
                    }
                }
                if (let Some(superPkgCfg) <- task.superPkgCfg) {
                    for (link in superPkgCfg.subPkgSet) {
                        if (!taskMap.contains(link)) {
                            continue
                        }
                        if(!macroSet.contains(link)) { // 不需要 link 宏包
                            if (!rDeps.contains(link + ".ph2")) {
                                rDeps.add(link + ".ph2", HashSet<String>())
                            }
                            rDeps[link + ".ph2"].add(pkgName + ".ph2")
                            linkSet.add(link)
                        }
                    }
                }
                let cnt = linkSet.size
                sems.add(pkgName + ".ph2", SyncCounter(cnt + 1))
                println("sems "+pkgName + ".ph2"+": "+(cnt + 1).toString())
            } else {
                sems.add(pkgName + ".ph2", SyncCounter(1))
                println("sems "+pkgName + ".ph2"+": 1")
            }
        }

        let monitorFuture = spawn {
            =>
            monitor(macroSet, cjo2pkg, listenDirs, notify)
        }

        println("isProjectCombine: " + this.buildConfig.globalConfig.isProjectCombine.toString())

        // Guaranteed valid order follows 'buildConfig.packageList.all'
        for (pkgName in this.buildConfig.packageList.all) {
            if (!this.taskMap.contains(pkgName)) {
                continue
            }

            sem.acquire()
            let task = this.taskMap[pkgName]
            let future = spawn {
                =>
                if (this.buildConfig.globalConfig.isProjectCombine && pkgName == buildConfig.globalConfig.rootName) {
                    for ((k,_) in this.taskMap) {
                        if (k != pkgName && !this.checkDepFuture(k, task, futTasks, sem)) {
                            res.add(pkgName, false)
                            notify(pkgName)
                            notifyPh2(pkgName)
                            return false
                        }
                    }
                } else {
                    sems[pkgName].waitUntilZero()
                    for (dep in task.requirePkgs) {
                        if (res.contains(dep) && !res[dep]) {
                            res.add(pkgName, false)
                            notify(pkgName)
                            notifyPh2(pkgName)
                            return false
                        }
                    }

                    if (let Some(superPkgCfg) <- task.superPkgCfg) {
                        for (dep in superPkgCfg.subPkgSet) {
                            if (res.contains(dep) && !res[dep]) {
                                res.add(pkgName, false)
                                notify(pkgName)
                                notifyPh2(pkgName)
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
                        this.queue.add((None, None, None, None, false))
                        res.add(pkgName, true)
                        notify(pkgName)
                        notifyPh2(pkgName)
                        return true
                    }
                }

                if (!deleteLog(task)) {
                    sem.release()
                    this.queue.add((task.outLogFile, task.errLogFile, None, None, false))
                    res.add(pkgName, false)
                    notify(pkgName)
                    notifyPh2(pkgName)
                    return false
                }

                var flag: Bool = true
                if (task.targetPlatform && !runTask(getCommandArgs(task, this.buildConfig, false, false), task, this.buildConfig.globalConfig.ldPath)) {
                    flag = false
                }
                if (task.nativePlatform && !runTask(getCommandArgs(task, this.buildConfig, true, false), task, this.buildConfig.globalConfig.ldPath,
                    isNativeForCross: true)) {
                    flag = false
                }
                flag = true //temp
                sem.release()
                if (task.targetPlatform && task.nativePlatform) {
                    this.queue.add((task.outLogFile, task.errLogFile, task.nativeOutLog, task.nativeErrLog, true))
                } else if (task.targetPlatform) {
                    this.queue.add((task.outLogFile, task.errLogFile, None, None, true))
                } else if (task.nativePlatform) {
                    this.queue.add((None, None, task.nativeOutLog, task.nativeErrLog, true))
                }
                res.add(pkgName, flag)
                notify(pkgName)
                notifyPh2(pkgName)
                return flag
            }

            synchronized(this.buildMutex) {
                futTasks.add(pkgName, future)
            }

            sem.acquire()
            let future2 = spawn {
                =>
                let outputType = task.outputType.toString()
                let needLink = outputType == "dynamic" || outputType == "executable"
                if (needLink) {
                    println(pkgName+" need link")
                    let outputdir = if (task.nativePlatform) {task.nativePath} else {task.targetPath}
                    let linkpath = outputdir + "/" + pkgName + ".link"
                    sems[pkgName + ".ph2"].waitUntilZero()
                    if (exists(linkpath)) {
                        let content = String.fromUtf8(File.readFrom(linkpath))
                        let array = content.split(' ')
                        var args = ArrayList<String>()
                        for (item in array) {
                            let res = item.removePrefix("'").removeSuffix("'")
                            if (!res.isEmpty()) {
                                args.add(res)
                            }
                        }
                        let tool = args[0]
                        args.remove(at: 0)
                        var outFile: File
                        var errFile: File
                        try {
                            if (task.targetPlatform) {
                                outFile = File(task.outLogFile, OpenMode.Append)
                                errFile = File(task.errLogFile, OpenMode.Append)
                            } else {
                                outFile = File(task.nativeOutLog, OpenMode.Append)
                                errFile = File(task.nativeErrLog, OpenMode.Append)
                            }
                        } catch (e: Exception) {
                            eprintln(e.message)
                            return false
                        }
                        if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
                            addStartTime(task.fullName + ".ph2", "link")
                        }
                        if (let Some(returnCode) <- execAndToFile(tool, args, outFile, errFile)) {
                            if (returnCode == 0) {
                                println("success to link for `${task.fullName}`")
                            } else {
                                println("Error: failed to link for `${task.fullName}`, return code is ${returnCode}")
                            }
                        }
                        if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
                            addEndTime(task.fullName + ".ph2")
                        }
                        outFile.close()
                        errFile.close()
                    } else {
                        println(pkgName+" no link file")
                    }
                }
                else {
                    println(pkgName+" need not link")
                }
                sem.release()
                notify(pkgName+".ph2")
                true
            }
            synchronized(this.buildMutex) {
                futTasks.add(pkgName + "ph2", future2)
            }
        }
        var isBuilt: Bool = true
        for ((pkgName, task) in futTasks) {
            println(pkgName+" not donelz")
            if (!task.get()) {
                isBuilt = false
            } else {
                successPkgs.add(pkgName)
            }
            println(pkgName+" has donelz")
        }

        println(" mkk 1")
        // if (!logFuture.get()) {
        //     isBuilt = false
        // }
        println(" mkk 2")
        return isBuilt
    }

    func deleteLog(task: ResolveConfig): Bool {
        var delLog: Bool = true
        if (task.targetPlatform) {
            if (!deleteFile(task.outLogFile)) {
                delLog = false
            }
            if (!deleteFile(task.errLogFile)) {
                delLog = false
            }
        }

        if (task.nativePlatform) {
            if (!deleteFile(task.nativeOutLog)) {
                delLog = false
            }
            if (!deleteFile(task.nativeErrLog)) {
                delLog = false
            }
        }
        return delLog
    }

    func runTask(args: ArrayList<String>, resolveConfig: ResolveConfig, ldPath: HashSet<String>, isNativeForCross!: Bool = false): Bool {
        let envBuilder = EnvironmentBuilder()
        envBuilder.prepend(LD_PATH, ldPath)

        let cjcCalls = ArrayList<ArrayList<String>>()
        if (resolveConfig.packagePath.isMultiplatform) {
            let (commonArgs, platformArgs) = getMultiplatformPipeline(resolveConfig, this.buildConfig, isNativeForCross)
            cjcCalls.add(commonArgs)
            cjcCalls.add(platformArgs)
        } else {
            args.add(all: getTypeCommand(resolveConfig, this.buildConfig, isNativeForCross))
            args.add("--dry-link")
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
                if (!isNativeForCross) {
                    if (!createAndWriteFile(resolveConfig.outLogFile, verbose, mode: Append)) {
                        return false
                    }
                } else {
                    if (!createAndWriteFile(resolveConfig.nativeOutLog, verbose, mode: Append)) {
                        return false
                    }
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
                if (!isNativeForCross) {
                    outFilePath = resolveConfig.outLogFile
                    errFilePath = resolveConfig.errLogFile
                    outFile = File(resolveConfig.outLogFile, OpenMode.Append)
                    errFile = File(resolveConfig.errLogFile, OpenMode.Append)
                } else {
                    outFilePath = resolveConfig.nativeOutLog
                    errFilePath = resolveConfig.nativeErrLog
                    outFile = File(resolveConfig.nativeOutLog, OpenMode.Append)
                    errFile = File(resolveConfig.nativeErrLog, OpenMode.Append)
                }
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
                        if (resolveConfig.packagePath.isMultiplatform) {
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

func parallelBuild(res: ModuleResolve, buildConfig: BuildConfig): Bool {
    println("parallelBuild lz")
    let parallel = ParallelBuilder(buildConfig)
    res.forEachPkg(
        {
            resolve: ResolveItem =>
            if (buildConfig.isIncremental && !buildConfig.rebuildList.contains(resolve.fullName)) {
                return true
            }

            if (!resolve.hasProdFiles) { // No files to compile, avoid useless cjc invocations
                buildConfig.rebuildList.remove(resolve.fullName)
                return true
            }

            var finalPath = getFinalPath(buildConfig.globalConfig.targetDir, buildConfig.globalConfig.nativeDir,
                resolve.outputType, true, resolve.rootPkgName, buildConfig.requiredForTests)
            var finalLogPath = getLogPath(buildConfig.globalConfig.targetDir, buildConfig.globalConfig.nativeDir, true,
                resolve.rootPkgName)
            if (!createDirectory(finalPath) || !createDirectory(finalLogPath)) {
                return false
            }

            if (resolve.packagePath.isMultiplatform) {
                let finalCommonPath = Path(finalPath).join("common").toString()
                if (!createDirectory(finalCommonPath)) {
                    return false
                }
            }

            var resolveConfig = ResolveConfig()
            resolveConfig.targetPath = finalPath
            resolveConfig.packagePath = resolve.packagePath
            resolveConfig.fullName = resolve.fullName
            resolveConfig.rootPkgName = resolve.rootPkgName
            resolveConfig.outputType = resolve.outputType
            resolveConfig.compileOption = resolve.compileOption
            resolveConfig.linkOption = resolve.linkOption
            resolveConfig.nativeCompileOption = resolve.nativeCompileOption
            resolveConfig.nativeLinkOption = resolve.nativeLinkOption
            resolveConfig.customizedOption = resolve.customizedOption
            resolveConfig.requirePkgs = resolve.requires
            resolveConfig.targetPlatform = resolve.targetPlatform
            resolveConfig.nativePlatform = resolve.nativePlatform
            resolveConfig.superPkgCfg = resolve.superPkgCfg
            resolveConfig.hasTestFiles = resolve.hasTestFiles
            if (buildConfig.isIncremental && resolveConfig.outputType == Static &&
                buildConfig.indirectRebuilds.contains(resolve.fullName)) {
                resolveConfig.checkDepCjo = true
            }
            resolveConfig.outLogFile = Path(finalLogPath).join("${resolveConfig.fullName}.outlog").toString()
            resolveConfig.errLogFile = Path(finalLogPath).join("${resolveConfig.fullName}.errlog").toString()

            if (resolve.nativePlatform) {
                resolveConfig.nativePath = getFinalPath(buildConfig.globalConfig.targetDir,
                    buildConfig.globalConfig.nativeDir, resolve.outputType, false, resolve.rootPkgName,
                    buildConfig.requiredForTests)
                let nativeLogPath = getLogPath(buildConfig.globalConfig.targetDir, buildConfig.globalConfig.nativeDir,
                    false, resolve.rootPkgName)
                resolveConfig.nativeOutLog = Path(nativeLogPath).join("${resolveConfig.fullName}.outlog").toString()
                resolveConfig.nativeErrLog = Path(nativeLogPath).join("${resolveConfig.fullName}.errlog").toString()
                if (!createDirectory(resolveConfig.nativePath) || !createDirectory(nativeLogPath)) {
                    return false
                }
            }

            parallel.taskMap.add(resolve.fullName, resolveConfig)
            return true
        }
    )

    if (!parallel.taskMap.isEmpty()) {
        buildConfig.isRebuild = true
    }

    let parallelResult: Bool = parallel.build()
    println(" mkk 3")
    for (r in res.resolves) {
        if (parallel.successPkgs.contains(r.fullName)) {
            r.buildFlag = true
        }
    }

    return parallelResult
}

func printAllLogs(outLogFile: Option<String>, errLogFile: Option<String>, nativeOutLog: Option<String>,
    nativeErrLog: Option<String>): Bool {
    var logFlag: Bool = true
    if (let Some(outLogFile) <- outLogFile) {
        if (fileExists(outLogFile) && !loadLogAndPrint(outLogFile, false)) {
            logFlag = false
        }
    }
    if (let Some(errLogFile) <- errLogFile) {
        if (fileExists(errLogFile) && !loadLogAndPrint(errLogFile, true)) {
            logFlag = false
        }
    }
    if (let Some(nativeOutLog) <- nativeOutLog) {
        if (fileExists(nativeOutLog) && !loadLogAndPrint(nativeOutLog, false)) {
            logFlag = false
        }
    }
    if (let Some(nativeErrLog) <- nativeErrLog) {
        if (fileExists(nativeErrLog) && !loadLogAndPrint(nativeErrLog, true)) {
            logFlag = false
        }
    }
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

func getFinalPath(targetDir: String, nativeDir: String, outputType: OutputType, isTargetPlatform: Bool, name: String,
    requiredForTests: Bool): String {
    var res = ""
    if (outputType == Exe && !requiredForTests) {
        if (isTargetPlatform) {
            res = Path(targetDir).join(BIN).toString()
        } else {
            res = Path(nativeDir).join(BIN).toString()
        }
    } else {
        if (isTargetPlatform) {
            res = Path(targetDir).join(name).toString()
        } else {
            res = Path(nativeDir).join(name).toString()
        }
    }
    return res
}

func getLogPath(targetDir: String, nativeDir: String, isTargetPlatform: Bool, rootPkgName: String): String {
    var res = ""
    if (isTargetPlatform) {
        res = Path(targetDir).join(".build-logs").join(rootPkgName).toString()
    } else {
        res = Path(nativeDir).join(".build-logs").join(rootPkgName).toString()
    }
    return res
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
