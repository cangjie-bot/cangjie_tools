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

type TaskMap = HashMap<TaskId, CompileTask>

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

// class ParallelBuilder {
//     var buildConfig = BuildConfig()
//     var taskMap = TaskMap()
//     let buildMutex = Mutex()
//     let successPkgs = HashMap<String, Bool>()
// 
//     init(buildConfig: BuildConfig) {
//         this.buildConfig = buildConfig
//     }
// 
//     func checkDepFuture(dep: TaskId, task: CompileTask, futTasks: HashMap<TaskId, Future<Bool>>,
//         sem: Semaphore): Bool {
//         var fut: Future<Bool>
//         synchronized(this.buildMutex) {
//             if (!futTasks.contains(dep)) {
//                 return true
//             }
//             fut = futTasks[dep]
//         }
//         return fut.get()
//     }
// 
//     func build(): Bool {
//         let logger = BoundedFileLogger(this.taskMap.size)
//         logger.start()
// 
//         let sem = Semaphore(maxParallelSize)
//         let futTasks = HashMap<TaskId, Future<Bool>>()
// 
//         // TODO: remove later
//         let productSuffixes = buildConfig.packageList.productSuffixes
// 
//         // Guaranteed valid order follows 'buildConfig.packageList.all'
//         for (taskId in this.buildConfig.packageList.allIds) {
//             if (!this.taskMap.contains(taskId)) {
//                 continue
//             }
// 
//             sem.acquire()
//             let task = this.taskMap[taskId]
//             let future = spawn { =>
//                 if (!createDirectory(task.targetPath) || !createDirectory(task.logPath)) {
//                     sem.release()
//                     logger.release()
//                     return false
//                 }
// 
//                 if (this.buildConfig.globalConfig.isProjectCombine && taskId.fullName == buildConfig.globalConfig.rootName) {
//                     for ((k,_) in this.taskMap where k != taskId) {
//                         if (!this.checkDepFuture(k, task, futTasks, sem)) {
//                             sem.release()
//                             logger.release()
//                             return false
//                         }
//                     }
//                 } else {
//                     for (dep in task.requireTasks) {
//                         if (!this.checkDepFuture(dep, task, futTasks, sem)) {
//                             sem.release()
//                             logger.release()
//                             return false
//                         }
//                     }
// 
//                     if (let Some(superPkgCfg) <- task.superPkgCfg) {
//                         for (dep in superPkgCfg.subPkgSet) {
//                             let depId = TaskId.Compile(dep, task.target, productSuffixes[dep])
//                             if (!this.checkDepFuture(depId, task, futTasks, sem)) {
//                                 sem.release()
//                                 logger.release()
//                                 return false
//                             }
//                         }
//                     }
//                 }
// 
//                 if (!deleteLog(task)) {
//                     sem.release()
//                     logger.release()
//                     return false
//                 }
// 
//                 let cjcCall = constructCjcInvocation(task, this.buildConfig)
//                 let envBuilder = EnvironmentBuilder()
//                 envBuilder.prepend(LD_PATH, this.buildConfig.globalConfig.ldPath)
//                 let flag = runTask(task, cjcCall, envBuilder) 
// 
//                 sem.release()
//                 logger.log(task.outLogFile, task.errLogFile)
//                 return flag
//             }
// 
//             synchronized(this.buildMutex) {
//                 futTasks.add(taskId, future)
//             }
//         }
// 
//         var isBuilt: Bool = true
//         for ((id, task) in futTasks) {
//             if (!task.get()) {
//                 isBuilt = false
//                 successPkgs.add(id.fullName, false)
//             } else if (!successPkgs.contains(id.fullName)) {
//                 successPkgs.add(id.fullName, true)
//             }
//         }
// 
//         if (!logger.unsafeWait()) {
//             isBuilt = false
//         }
// 
//         return isBuilt
//     }
// 
//     func deleteLog(task: CompileTask): Bool {
//         var delLog: Bool = true
//         if (!deleteFile(task.outLogFile)) {
//             delLog = false
//         }
//         if (!deleteFile(task.errLogFile)) {
//             delLog = false
//         }
//         return delLog
//     }
// 
//     func runTask(compileTask: CompileTask, args: ArrayList<String>, envBuilder: EnvironmentBuilder): Bool {
//         var execCmdFlag: Bool = true
//         CUR_PARALLEL_SIZE.fetchAdd(1)
//         if (!futJudge(args)) {
//             let jobs = calculateParallel(CUR_PARALLEL_SIZE.load())
//             args.add("-j${jobs}", at: 0)
//         }
// 
//         let env = getVariables()
//         let commandStr = getCmdStr(envBuilder.asCliStrings(env), COMPILE_TOOL, args)
//         if (this.buildConfig.isVerbose) {
//             let verbose: String = if (compileTask.isMultiplatform) {
//                 "Compiling `${compileTask.sourceSetDir}` part of package `${compileTask.fullName}`: ${commandStr}\n"
//             } else { 
//                 "Compiling package `${compileTask.fullName}`: ${commandStr}\n"
//             }
//             if (!createAndWriteFile(compileTask.outLogFile, verbose, mode: Append)) {
//                 return false
//             }
//         }
// 
//         if (compileTask.isAnalysisCompilePerformance) {
//             addStartTime(compileTask.fullName, commandStr)
//         }
// 
//         let outFilePath = compileTask.outLogFile
//         let errFilePath = compileTask.errLogFile
//         let (outFile, errFile) = try {
//             (File(outFilePath, OpenMode.Append), File(errFilePath, OpenMode.Append))
//         } catch (e: Exception) {
//             eprintln(e.message)
//             eprintln("Error: create '${outFilePath}' failed")
//             return false
//         }
// 
//         try {
//             if (let Some(returnCode) <- execAndToFile(COMPILE_TOOL, args, outFile, errFile, envBuilder: envBuilder, originalEnv: env)) {
//                 execCmdFlag = (returnCode == 0)
//                 if (execCmdFlag) {
//                     if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
//                         moveCjcProfToCjpmDir(compileTask, buildConfig.globalConfig.compilePerformanceTargetDir)
//                     }
//                 } else {
//                     let errmesg = if (compileTask.isMultiplatform) {
//                         "Error: failed to compile `${compileTask.sourceSetDir}` part of package `${compileTask.fullName}`, return code is ${returnCode}\n"
//                     } else {
//                         "Error: failed to compile package `${compileTask.fullName}`, return code is ${returnCode}\n"
//                     }
//                     errFile.write(errmesg.toArray())
//                 }
//             } else {
//                 errFile.write("Error: failed to compile package `${compileTask.fullName}` with exception occurred\n".toArray())
//             }
//         } catch (e: Exception) {
//             eprintln("Error: failed to write error log into ${errFilePath}: ${e.message}")
//             execCmdFlag = false
//         }
//         CUR_PARALLEL_SIZE.fetchSub(1)
//         outFile.close()
//         errFile.close()
// 
//         if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
//             addEndTime(compileTask.fullName)
//         }
//         return execCmdFlag
//     }
// }

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
