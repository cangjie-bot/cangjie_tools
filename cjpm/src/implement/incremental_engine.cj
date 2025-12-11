// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import cjpm.util.*
import cjpm.config.*

import std.fs.*
import std.env.*
import std.sync.*
import std.collection.*
import std.collection.concurrent.*
import std.deriving.*

import stdx.encoding.json.*
import stdx.serialization.serialization.*

// TYPES
public type Hash = Int64

/**
 * Non-cacheable data (root inputs, always computed)
 */
public interface IncData {
    /**
     * case None  - Although, value is known, the current state is invalid and the task should be recomputed
     * case value - The value is valid, and can be reused
     */
    func incHash(): ?Hash
}

/**
 * Cacheable data
 */
public interface SerializableIncData<T> <: IncData where T <: Serializable<T> { 
    /**
     * case None  - Although, value is known, the current state is invalid and the task should be recomputed
     * case value - The value is valid, and can be reused
     */
    func incHash(): ?Hash
}

// TASK OUTPUTS
public struct PackageSources <: Serializable<PackageSources> {
    PackageSources(
        public let dirpath: Path
    ) { }

    public func serialize(): DataModel {
        dirpath.toString().serialize()
    }

    public static func deserialize(dm: DataModel): PackageSources {
        let dmstr = dm as DataModelString ?? throw DataModelException("this data is not DataModelString")
        PackageSources(Path(dmstr.getValue()))
    }
}

extend PackageSources <: SerializableIncData<PackageSources> {
    public func incHash(): ?Hash { 
        if (exists(dirpath) && FileInfo(canonicalize(dirpath)).isDirectory()) {
            return Directory.readFrom(dirpath) |> 
                filter { it => it.isRegular() } |>
                map { it => it.lastModificationTime } |>
                unorderedHash
        }
        return None 
    }
}


public enum ArtifactCheckKind <: ToString & Serializable<ArtifactCheckKind> {
    | CheckExists
    | CheckTimestamp

    public func toString(): String {
        match (this) {
            case CheckExists => "check-exists"
            case CheckTimestamp => "check-timestamp"
        }
    }

    public func serialize(): DataModel {
        DataModelString(this.toString())
    }

    public static func fromString(value: String): ArtifactCheckKind {
        match (value) {
            case "check-exists" => CheckExists
            case "check-timestamp" => CheckTimestamp
            case _ => throw Exception("Unknown kind of aftifact checking: `${value}`")
        }
    }

    public static func deserialize(dm: DataModel): ArtifactCheckKind {
        let dmstr = match (dm) {
            case dmstr: DataModelString => dmstr
            case _ => throw DataModelException("this is not DataModelString")
        }
        fromString(dmstr.getValue())
    }
}

@Derive[ToString]
public struct Artifact <: Serializable<Artifact> {
    private static let PATH_FIELD = "path"
    private static let CHECK_KIND_FIELD = "checkKind"
    Artifact(
        public let path: String,
        public let checkKind: ArtifactCheckKind
    ) { }

    public func serialize(): DataModel {
        DataModelStruct()
            .add(field(PATH_FIELD, path))
            .add(field(CHECK_KIND_FIELD, checkKind))
    }

    public static func deserialize(dm: DataModel): Artifact {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw DataModelException("this data is not DataModelStruct")
        }
        let path = String.deserialize(dms.get(PATH_FIELD))
        let checkKind = ArtifactCheckKind.deserialize(dms.get(CHECK_KIND_FIELD))
        Artifact(path, checkKind)
    }
}

extend Artifact <: SerializableIncData<Artifact> {
    public func incHash(): ?Hash {
        match (checkKind) {
            case CheckExists => 
                if (!exists(path)) { return None } else { return 0 }
            case CheckTimestamp => 
                if (!exists(path)) { 
                    return None
                }

                try {
                    FileInfo(canonicalize(path)).lastModificationTime.hashCode()
                } catch (_: Exception) {
                    return None
                }
        }  
    }
}

@Derive[ToString]
public struct ArtifactGroup <: Serializable<ArtifactGroup> {
    ArtifactGroup(
        public let artifacts: Array<Artifact>
    ) { }

    public func serialize(): DataModel {
        artifacts.serialize()
    }

    public static func deserialize(dm: DataModel): ArtifactGroup {
        let dmseq = dm as DataModelSeq ?? throw DataModelException("this data is not DataModelSeq")
        ArtifactGroup(Array<Artifact>.deserialize(dmseq))
    }
}

extend ArtifactGroup <: SerializableIncData<ArtifactGroup> {
    public func incHash(): ?Hash {
        let size = artifacts.size
        let checked = artifacts.filterMap { it => it.incHash() } |> collectArray

        if (checked.size < size) {
            return None
        }

        checked |> unorderedHash
    }
}

// EXCEPTIONS
sealed abstract class IncrementalEngineException <: Exception {
    IncrementalEngineException(msg: String) { super(msg) }
}

public class TaskNotFoundException <: IncrementalEngineException {
    public TaskNotFoundException(id: TaskId) {
        super("Couldn't find task with id `${id}`")
    }
}

public class AmbiguousTaskException <: IncrementalEngineException {
    public AmbiguousTaskException(id: TaskId) {
        super("Task `${id}` is already defined")
    }
}

public class FinalizedBuildException <: IncrementalEngineException {
    public FinalizedBuildException() {
        super("IncrementalEngine object is finalized and cannot be changed")
    }
}

public class DataMistypeException <: IncrementalEngineException { 
    public DataMistypeException(id: TaskId) { 
        super("Task `${id}` stores the value of wrong type")
    }
}

public class NotComputedInputException <: IncrementalEngineException {
    public NotComputedInputException(id: TaskId) {
        super("Tried to get value from task `${id}` before calling `engine.execute()`")
    }
}

public class MissingArtifactException <: IncrementalEngineException {
    public MissingArtifactException(id: TaskId) {
        super("Task `${id}` was executed, but artifact didn't get produced or got missing")
    } 
}

public class TaskComputeException <: IncrementalEngineException {
    public TaskComputeException(id: TaskId, error: Exception) {
        super("Task `${id}` threw an exception:\n${error}")
    }
}

// TASK GRAPH
class TaskGraphBuilder {
    private let graph = HashMap<TaskId, (Array<TaskId>, ArrayList<TaskId>)>()
    private let rootTasks = HashSet<TaskId>()
    private let toposort = ArrayList<TaskId>()

    init() { }

    public func addTask(id: TaskId, dependencies!: Array<TaskId> = []): Unit {
        if (graph.contains(id)) {
            throw AmbiguousTaskException(id)
        }
        if (dependencies.size == 0) {
            this.rootTasks.add(id)
        }
        for (dep in dependencies) {
            let (_, out) = graph.get(dep) ?? throw TaskNotFoundException(dep)
            out.add(id)
        }
        graph.add(id, (dependencies, ArrayList()))
        toposort.add(id)
    }

    public func finish(): TaskGraph {
        let newGraph = graph |> map { it: (TaskId, (Array<TaskId>, ArrayList<TaskId>)) => (it[0], TaskNode(it[1][0], it[1][1].toArray())) } |> collectHashMap
        TaskGraph(newGraph, rootTasks.clone(), toposort.toArray())
    }
}

struct TaskNode {
    TaskNode(
        let indeg: Array<TaskId>,
        let outdeg: Array<TaskId>
    ) { }

    func filterEdges(cond: (TaskId) -> Bool): TaskNode {
        TaskNode(
            this.indeg |> filter(cond) |> collectArray,
            this.outdeg |> filter(cond) |> collectArray
        )
    }
}

class TaskGraph {
    TaskGraph(
        let graph: HashMap<TaskId, TaskNode>,
        let _rootTasks: HashSet<TaskId>,
        let _toposort: Array<TaskId>
    ) { }

    public prop toposort: Array<TaskId> {
        get() { _toposort }
    }

    public prop rootTasks: Array<TaskId> {
        get() { _rootTasks.toArray() }
    }

    public static func builder(): TaskGraphBuilder {
        TaskGraphBuilder()
    }

    public func filterGraph(cond: (TaskId) -> Bool): TaskGraph {
        let _rootTasks = _rootTasks |> filter(cond) |> collectHashSet
        let graph = graph |> filterMap { it: (TaskId, TaskNode) =>
            let (id, state) = it
            if (!cond(id)) { return Option<(TaskId, TaskNode)>.None }
            return (id, state.filterEdges(cond))
        } |> collectHashMap
        let toposort = toposort |> filter(cond) |> collectArray
        TaskGraph(graph, _rootTasks, toposort)
    }

    public func traverse(from: TaskId, onEntry: (TaskId) -> Unit) {
        traverse(from, { _ => true }, onEntry)
    }

    public func traverse(from: TaskId, entryCondition: (TaskId) -> Bool, onEntry: (TaskId) -> Unit) {
        let stack = ArrayDeque<TaskId>()
        let visited = HashSet<TaskId>()
        stack.addFirst(from)
        while (let Some(from) <- stack.removeFirst()) {
            visited.add(from)
            onEntry(from)
            for (item in graph[from].outdeg) {
                if (!visited.contains(item) && entryCondition(item)) {
                    stack.addLast(item)
                }
            }
        }
    }

    public func traverseReverse(from: TaskId, onEntry: (TaskId) -> Unit) {
        traverseReverse(from, { _ => true }, onEntry)
    }

    public func traverseReverse(from: TaskId, entryCondition: (TaskId) -> Bool, onEntry: (TaskId) -> Unit) {
        let stack = ArrayDeque<TaskId>()
        let visited = HashSet<TaskId>()
        stack.addFirst(from)
        while (let Some(from) <- stack.removeFirst()) {
            visited.add(from)
            onEntry(from)
            for (item in graph[from].indeg) {
                if (!visited.contains(item) && entryCondition(item)) {
                    stack.addLast(item)
                }
            }
        }
    }
}

extend TaskGraph {

    /**
     * Provide a view of graph which storing all dependencies, that potentially required to compute task `id`
     */
    public func askView(id: TaskId): TaskGraph {
        /**
         * Because `TaskGraph` is closed for addition of new vertices and egdes, 
         * we don't have to keep links to values stored in original graph
         */
        let asked = HashSet<TaskId>()
        traverseReverse(id) { it => asked.add(it) }
        this.filterGraph { it: TaskId => asked.contains(it) }
    }
}

extend TaskGraph {
    public func incoming(id: TaskId): Array<TaskId> {
        graph[id].indeg
    }

    public func outcoming(id: TaskId): Array<TaskId> {
        graph[id].outdeg
    }
}

// CACHING
private class CacheItem <: Serializable<CacheItem> {
    private static let VALUE = "value"
    private static let INPUT_HASH = "inputHash"
    private static let OUTPUT_HASH = "outputHash"

    CacheItem(
        let value: DataModel,
        let inputHash: Hash,
        let outputHash: Hash
    ) { }

    public func serialize(): DataModel {
        DataModelStruct()
            .add(Field(VALUE, value))
            .add(field(INPUT_HASH, inputHash))
            .add(field(OUTPUT_HASH, outputHash))
    }

    public static func deserialize(dm: DataModel): CacheItem {
        let dms = match (dm) {
            case dms: DataModelStruct => dms
            case _ => throw DataModelException("this data is not DataModelStruct")
        }
        let value = dms.get(VALUE)
        let inputHash = Hash.deserialize(dms.get(INPUT_HASH))
        let outputHash = Hash.deserialize(dms.get(OUTPUT_HASH))
        CacheItem(value, inputHash, outputHash)
    }
}

public class IncrementalCache {
    private let cache: ConcurrentHashMap<TaskId, CacheItem>

    public IncrementalCache(
        private let cacheLocation: Path
    ) { 
        this.cache = readCacheFromJson(cacheLocation)
    }

    public func update(id: TaskId, inputHash: Hash, outputHash: Hash, dm: DataModel): Unit {
        cache.add(id, CacheItem(dm, inputHash, outputHash))
    }

    public func readValue(id: TaskId): ?DataModel {
        cache.get(id)?.value
    }

    public func readInputHash(id: TaskId): ?Hash {
        cache.get(id)?.inputHash
    }

    public func readOutputHash(id: TaskId): ?Hash {
        cache.get(id)?.outputHash
    }

    private static func readCacheFromJson(cacheLocation: Path): ConcurrentHashMap<TaskId, CacheItem> {
        let dm = readJsonFileOrEmpty(cacheLocation)
        try {
            // stdx.serialization.serialization doesn't work properly with non-string keys
            let hashmap = HashMap<String, CacheItem>.deserialize(dm)
                .iterator().map {it => (TaskId.fromString(it[0]), it[1])} |>
                collectHashMap
            ConcurrentHashMap(hashmap)
        } catch (e: Exception) {
            ConcurrentHashMap()
        }
    }

    public func storeCache(): Unit {
        writeJsonToFile(cacheLocation, cache)
    }
}

private func readJsonFileOrEmpty(path: Path): DataModel {
    try {
        let barray = File.readFrom(path)
        let s = String.fromUtf8(barray)
        let json = JsonValue.fromStr(s)
        let res = DataModel.fromJson(json)
        return res
    } catch (e: Exception) {
        DataModel.fromJson(JsonObject())
    }
}

private func writeJsonToFile(path: Path, cache: ConcurrentHashMap<TaskId, CacheItem>): Unit {
    try {
        // stdx.serialization.serialization doesn't work properly with non-string keys
        let dm = (cache |> map { it => (it[0].toString(), it[1]) } |> collectHashMap).serialize()
        let json = dm.toJson()
        let s = json.toJsonString()
        let barray = s.toArray()
        File.writeTo(path, barray)
    } catch (e: Exception) { }
}

// TASKS
sealed abstract class ErasedTask {
    protected var _value: Option<IncData> = None
    public var logFiles: ?(String, String) = None

    ErasedTask(
        public let id: TaskId
    ) { }

    public func compute(): Unit
    public func cleanCompute(): Unit
    public func asDataModel(): DataModel
    public func computedHash(): Hash
    public func readValidCachedValue(): Bool
}

public class Task<T> <: ErasedTask where T <: SerializableIncData<T> {
    private let computation: Option<() -> T>
    private let cache: IncrementalCache

    public init(id: TaskId, cache: IncrementalCache, comp: () -> T) {
        super(id)
        this._value = None
        this.computation = comp
        this.cache = cache
    }

    mut prop value: Option<T> {
        get() {
            if (let Some(incData) <- _value) {
                match (incData) {
                    case v: T => v
                    case _ => throw DataMistypeException(id)
                }
            } else {
                Option<T>.None 
            }
        }
        set(newval) {
            if (let Some(v) <- newval) {
                this._value = v
            } else { this._value = None }
        }
    }

    public func computedHash(): Hash {
        if (let Some(res) <- value) {
            let hash = try { res.incHash() } catch (_: Exception) { throw NotComputedInputException(id) }
            return hash ?? throw MissingArtifactException(id)
        } else { throw NotComputedInputException(id) }
    }

    public func asDataModel(): DataModel {
        if (let Some(res) <- value) {
            let dm = try { res.serialize() } catch (_: Exception) { throw NotComputedInputException(id) }
            return dm
        } else { throw NotComputedInputException(id) }
    }

    public func compute(): Unit {
        if (this.value.isNone()) {
            cleanCompute()
        }
    }

    public func cleanCompute(): Unit {
        if (let Some(comp) <- computation) {
            this.value = try { 
                comp() 
            } catch (e: Exception) { 
                throw TaskComputeException(id, e) 
            }
        }
    }

    public func readValidCachedValue(): Bool {
        if (let Some(dm) <- cache.readValue(id)) {
            try {
                let result = T.deserialize(dm)
                if (let Some(storedHash) <- cache.readOutputHash(id) && result.incHash() == storedHash) {
                    this.value = result
                    return true
                }
            } catch(e: Exception) { 
                println(e)
            }
        } 
        return false
    }

    public operator func ()(): T {
        if (this.value.isNone()) {
            readValidCachedValue()
        }
        return this.value ?? throw NotComputedInputException(id)
    }
}

// INCREMENTAL ENGINE
public class IncrementalEngine {
    let taskGraphBuilder = TaskGraph.builder()
    let taskStorage = HashMap<TaskId, ErasedTask>()
    let dirtyTasks = HashSet<TaskId>()
    let cacheLocation: Path
    let cache: IncrementalCache

    private var finalized = false

    public IncrementalEngine(
        cacheLocation!: Path,
        private let jobs!: Int64 = 64
    ) { 
        this.cacheLocation = cacheLocation
        this.cache = IncrementalCache(cacheLocation)
    }

    func checkFinalized(): Unit {
        if (finalized) {
            throw FinalizedBuildException()
        }
    }

    public func askAll(): Unit {
        askHelper(None)
    }

    public func ask(taskId: TaskId): Unit {
        askHelper(taskId)
    }

    private func askHelper(taskId: ?TaskId) {
        checkFinalized()

        let graph = if (let Some(taskId) <- taskId) {
            taskGraphBuilder.finish().askView(taskId)
        } else {
            taskGraphBuilder.finish()
        } 

        for (id in graph.rootTasks where !dirtyTasks.contains(id)) {
            let input = taskStorage.get(id) ?? throw TaskNotFoundException(id)
            input.compute()
            if (let newHash <- input.computedHash() && cache.readOutputHash(id) != newHash) {
                cache.update(id, 0, newHash, input.asDataModel())
            }
        }

        for (id in graph.toposort where !dirtyTasks.contains(id)) {
            var isClean = true
            let dependencies = graph.incoming(id)
            isClean &&= dependencies |> all { it => !dirtyTasks.contains(it) }
            isClean &&= { => 
                let cachedDeps = dependencies |> filterMap { it => cache.readOutputHash(it) } |> collectArray
                if (dependencies.size != cachedDeps.size) { return false }
                let currentDepsHash = cachedDeps.iterator() |> unorderedHash
                let storedDepsHash  = cache.readInputHash(id)
                storedDepsHash == currentDepsHash }()
            isClean &&= taskStorage[id].readValidCachedValue()

            if (!isClean) {
                dirtyTasks.add(id)
            }
        }

    }

    // `dirtyGraph` - a subgraph of `graph` of tasks to be executed
    // `graph` - a whole graph of tasks. Provided for proper incrementality and cache updates
    private func executeSubgraph(dirtyGraph: TaskGraph, graph: TaskGraph): TaskResult {
        checkFinalized()

        let result = rebuild(taskStorage, dirtyGraph, graph, cache, jobs)
        cache.storeCache()

        finalized = true
        return result
    }

    public func execute(): TaskResult {
        let graph = taskGraphBuilder.finish()
        let dirtyGraph = graph.filterGraph { it: TaskId => dirtyTasks.contains(it) }
        executeSubgraph(dirtyGraph, graph)
    }

    public func executeClean(): TaskResult {
        let graph = taskGraphBuilder.finish()
        executeSubgraph(graph, graph)
    }

}

// TASK CREATION
private struct PrevStage {
    PrevStage(
        let chir!: String,
        let commonCjo!: String,
        let taskId!: TaskId
    ) { }
}

private func constructCustomizedOption(r: ResolveItem, buildConfig: BuildConfig): ArrayList<String> {
    let customizedOption = ArrayList<String>()
    if (buildConfig.customizedOption.size != 0) {
        for (k in buildConfig.customizedOption) {
            if (let Some(option) <- r.customizedOption.get(k)) {
                customizedOption.add(option)
            }
        }
    }
    return customizedOption
}

// Returns topologically sorted array of compiler calls, which are required to compile a package
private func createPackageCompileTasks(r: ResolveItem, buildConfig: BuildConfig, productOutputTypes: HashMap<String, OutputType>): ArrayList<CompileTask> {
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
    let isMacro = buildConfig.packageList.macros.contains(fullName)
    let superPkgCfg = r.superPkgCfg
    let isAnalysisCompilePerformance = !isMultiplatform && buildConfig.globalConfig.isAnalysisCompilePerformance

    let allEnabledFeatures = r.featureDeducer
        .addFeature(r.features)
        .collect()
    let featureMapping = r.featureDeducer.cleanFeatures() // leaving only mapping to apply them to source set features

    // Map from fullPkgName to sourceSetDir for creating task identifiers
    let productSuffixes = buildConfig.packageList.productSuffixes

    // Specific to the `--target` platform
    // `isNativeForCross == true` - package needs to be compiled with `host` target to be used by or is a `macro package` 
    func createForTarget(isNativeForCross!: Bool = false): ArrayList<CompileTask> {
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
        func createForSingleSourceSet(sourceSetEntry: CJMPPackageInfo, prevStage: ?PrevStage): (CompileTask, ?PrevStage) {
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

            let taskId = TaskId.Compile(fullName, target, outputType, sourceSetDir)

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

            // Requires is not only the compilation tasks. It's the names of all dependencies
            // We need to leave only packages we're creating compilation tasks for
            let requireTasks = r.requires.iterator()
                .filter { it => productOutputTypes.contains(it) }
                .map { it => 
                    let waitTarget = if (buildConfig.packageList.macros.contains(it)) {
                        assertion { r.nativePlatform }
                        targetConfigName
                    } else { target }
                TaskId.Compile(it, waitTarget, productOutputTypes[it], productSuffixes.get(it) ?? "") 
            } |> collectHashSet
            requireTasks.add(TaskId.ReadPackageSource(fullName, sourceSetDir))
            requireTasks.addIfSome(prevStage?.taskId)

            let prevStageChir = prevStage?.chir
            let prevStageCjo = prevStage?.commonCjo


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
                logPath: logPath,
                outLogFile: outLogFile,
                errLogFile: errLogFile
            )

            return (task, thisStage)
        }

        let sourceSetUnwrap = ArrayList<CompileTask>()
        var prevStage: ?PrevStage = None
        for (sourceSetEntry in r.packagePath._sources) {
            let (task, thisStage) = createForSingleSourceSet(sourceSetEntry, prevStage)
            sourceSetUnwrap.add(task)
            prevStage = thisStage
        }
        return sourceSetUnwrap
    }

    let result = ArrayList<CompileTask>()

    assertion { r.targetPlatform || r.nativePlatform }
    if (r.targetPlatform) {
        result.add(all: createForTarget(isNativeForCross: false))
    }
    if (r.nativePlatform) {
        result.add(all: createForTarget(isNativeForCross: true))
    }

    return result
}

// MAIN BUILD FUNCTION

// This functions creates an list of files, expected to be produced after compiler call
// Then different kinds of checks are performed over each file to choose whether to:
// - run task, which calls compiler
// - reuse artifacts from previous run
private func createArtifactGroup(compileTask: CompileTask, buildConfig: BuildConfig): ArtifactGroup {
    ArtifactGroup(
    match (compileTask.outputType) {
        case Static => 
            let name = if (compileTask.isLto) {
                makeLtoName(compileTask.filename)
            } else {
                makeCangjieStaticlibName(compileTask.filename)
            }
            [
                Artifact(Path(compileTask.targetPath).join(name).toString(), 
                    CheckExists),
                Artifact(Path(compileTask.targetPath).join("${swapOrgName(compileTask.fullName)}.cjo").toString(), 
                    CheckTimestamp)
            ]
        case Dynamic => 
            if (compileTask.isMacro) {[
                Artifact(Path(compileTask.targetPath).join(makeTargetMacroName(compileTask.filename, compileTask.target)).toString(),
                    CheckTimestamp),
                Artifact(Path(compileTask.targetPath).join("${swapOrgName(compileTask.fullName)}.cjo").toString(),
                    CheckTimestamp)
            ]} else {[
                Artifact(Path(compileTask.targetPath).join(makeTargetDylibName(compileTask.filename, compileTask.target)).toString(),
                    CheckExists),
                Artifact(Path(compileTask.targetPath).join("${swapOrgName(compileTask.fullName)}.cjo").toString(),
                    CheckTimestamp)
            ]}
        case Exe => [
            Artifact(Path(compileTask.targetPath).join(makeTargetExeName(compileTask.filename, compileTask.target)).toString(),
                CheckExists),
            Artifact(Path(compileTask.targetPath).join("${swapOrgName(compileTask.fullName)}.cjo").toString(),
                CheckTimestamp)
        ]
        case Chir => [
            Artifact(Path(compileTask.targetPath).join("${swapOrgName(compileTask.fullName)}.chir").toString(),
                CheckTimestamp),
            Artifact(Path(compileTask.targetPath).join("${swapOrgName(compileTask.fullName)}.cjo").toString(),
                CheckTimestamp)
        ]
        case _ => []
    })
}

func parallelBuild(module: ModuleResolve, buildConfig: BuildConfig): Bool {
    let engine = IncrementalEngine(
        cacheLocation: Path(buildConfig.globalConfig.targetDir).join(INCREMENTAL_CACHE),
        jobs: maxParallelSize
    )

    // The `--output-type` for `exe` files is set to `staticlib` when compiling it via `cjpm test`
    // So the output type has to be a part of task identifier
    let productOutputTypes = HashMap<String, OutputType>()

    // module.resolves is topologically sorted
    module.resolves.forEach { r: ResolveItem =>
        println("Creating tasks for resolve item: ${r.fullName}")
        let compileTasks = createPackageCompileTasks(r, buildConfig, productOutputTypes)
        let sourceTasks = HashSet<TaskId>()
        for (compileTask in compileTasks) {
            if (compileTask.product) {
                productOutputTypes.add(compileTask.fullName, compileTask.outputType)
            }
            let readSourceTaskId = TaskId.ReadPackageSource(compileTask.fullName, compileTask.sourceSetDir)

            if (!sourceTasks.contains(readSourceTaskId)) {
                sourceTasks.add(readSourceTaskId)
                engine.taskGraphBuilder.addTask(readSourceTaskId, dependencies: [])
                let out = Task(readSourceTaskId, engine.cache) { => 
                    return PackageSources(Path(compileTask.packagePath))
                }
                engine.taskStorage.add(readSourceTaskId, out)
            }
            engine.taskGraphBuilder.addTask(compileTask.id, dependencies: compileTask.requireTasks.toArray())

            let artifacts = createArtifactGroup(compileTask, buildConfig)
            var out = Task(compileTask.id, engine.cache) { =>
                if (!createDirectory(compileTask.targetPath)) {
                    throw Exception("Failed to create directory: `${compileTask.targetPath}`")
                }
                if (!createDirectory(compileTask.logPath)) {
                    throw Exception("Failed to create directory: `${compileTask.logPath}`")
                }
                let cjcCall = constructCjcInvocation(compileTask, buildConfig)
                let envBuilder = EnvironmentBuilder()
                envBuilder.prepend(LD_PATH, buildConfig.globalConfig.ldPath)
                if (!runTask(compileTask, cjcCall, envBuilder, buildConfig)) {
                    throw Exception("Failed to run task: `${compileTask.id}`")
                }
                return artifacts
            } 
            out.logFiles = (compileTask.outLogFile, compileTask.errLogFile)
            engine.taskStorage.add(compileTask.id, out)
        }
    }

    engine.askAll()

    if (engine.dirtyTasks.isEmpty()) {
        buildConfig.isRebuild = true
    }

    let result = if (buildConfig.isIncremental) {
        engine.execute()
    } else {
        engine.executeClean()
    }

    return match (result) {
        case Rebuild | Cached => true
        case Fatal => false
    }
}

// REBUILDER
@Derive[Equatable, ToString]
public enum TaskResult {
    | Cached
    | Rebuild
    | Fatal

    operator func +(b: TaskResult): TaskResult {
        match ((this, b)) {
            case (Fatal, _) => Fatal
            case (_, Fatal) => Fatal
            case (Rebuild, _) => Rebuild
            case (_, Rebuild) => Rebuild
            case (Cached, Cached) => Cached
        }
    }
}

extend Array<TaskResult> {
    public prop cached: Bool {
        get() {
            this.iterator().all { it => it == Cached }
        }
    }

    public prop fatal: Bool {
        get() {
            this.iterator().any { it => it == Fatal }
        }
    }
}

func deleteLog(logFiles: ?(String, String)): Bool {
    var delLog: Bool = true
    if (let Some((outLogFile, errLogFile)) <- logFiles) {
        if (!deleteFile(outLogFile)) {
            delLog = false
        }
        if (!deleteFile(errLogFile)) {
            delLog = false
        }
    }
    return delLog
}

func rebuild(taskStorage: ReadOnlyMap<TaskId, ErasedTask>, dirtyGraph: TaskGraph, taskGraph: TaskGraph, cache: IncrementalCache, jobs: Int64): TaskResult {
    let tasksCount = dirtyGraph.toposort.size
    let semaphore = Semaphore(jobs)
    let futureMap = ConcurrentHashMap<TaskId, Future<TaskResult>>(tasksCount)

    let logger = BoundedFileLogger(tasksCount)
    logger.start()

    for (id in dirtyGraph.toposort) {
        semaphore.acquire()
        let task = taskStorage[id]
        let future = spawn { =>
            let dependenciesResult = dirtyGraph.incoming(id)
                .map { it => futureMap[it].get() }

            if (dependenciesResult.fatal) {
                logger.release()
                semaphore.release()
                return Fatal
            }

            let dependencies = taskGraph.incoming(id)
            deleteLog(task.logFiles)
            try {
                // Early cutoff
                if (dependenciesResult.cached) {
                    var isClean = true
                    isClean &&= { => 
                        let cachedDeps = dependencies |> filterMap { it => cache.readOutputHash(it) } |> collectArray
                        if (dependencies.size != cachedDeps.size) { return false }
                        let currentDepsHash = cachedDeps.iterator() |> unorderedHash
                        let storedDepsHash  = cache.readInputHash(id)
                        storedDepsHash == currentDepsHash }()
                    isClean &&= taskStorage[id].readValidCachedValue()
                    if (isClean) {
                        logger.release()
                        semaphore.release()
                        return Cached
                    }
                }

                let newInputCache = dependencies |> filterMap { it: TaskId => cache.readOutputHash(it) } |> unorderedHash
                task.cleanCompute()
                if (let Some(hash) <- cache.readOutputHash(id)) {
                    if (task.computedHash() == hash) {
                        // Early cutoff
                        cache.update(id, newInputCache, task.computedHash(), task.asDataModel())
                        logger.log(task.logFiles)
                        semaphore.release()
                        return Cached
                    } else {
                        cache.update(id, newInputCache, task.computedHash(), task.asDataModel())
                        logger.log(task.logFiles)
                        semaphore.release()
                        return Rebuild
                    }
                } else {
                    cache.update(id, newInputCache, task.computedHash(), task.asDataModel())
                    logger.log(task.logFiles)
                    semaphore.release()
                    return Rebuild
                }
            } catch (e: IncrementalEngineException) {
                println(e)
                logger.log(task.logFiles)
                semaphore.release()
                return Fatal
            }
        }

        futureMap[id] = future
    }

    var result = Cached
    for ((_, future) in futureMap.iterator()) {
        result += future.get()
    }

    if (!logger.unsafeWait()) {
        result = Fatal
    }

    return result
}

private func runTask(compileTask: CompileTask, args: ArrayList<String>, envBuilder: EnvironmentBuilder, buildConfig: BuildConfig): Bool {
    var execCmdFlag: Bool = true
    CUR_PARALLEL_SIZE.fetchAdd(1)
    if (!futJudge(args)) {
        let jobs = calculateParallel(CUR_PARALLEL_SIZE.load())
        args.add("-j${jobs}", at: 0)
    }

    let env = getVariables()
    let commandStr = getCmdStr(envBuilder.asCliStrings(env), COMPILE_TOOL, args)
    if (buildConfig.isVerbose) {
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
