package cjpm.engine.core

import std.collection.*
import std.fs.*

public class IncrementalEngine {
    let taskGraphBuilder = TaskGraph.builder()
    let taskStorage = HashMap<TaskId, ErasedTask>()
    let dirtyTasks = HashSet<TaskId>()
    let cache: IncrementalCache

    private var finalized = false

    public IncrementalEngine(
        cacheLocation!: Path,
        private let jobs!: Int64 = 64
    ) { 
        this.cache = IncrementalCache(cacheLocation)
    }

    func checkFinalized(): Unit {
        if (finalized) {
            throw FinalizedBuildException()
        }
    }

    public func ask(taskId: TaskId): Unit {
        checkFinalized()

        let graph = taskGraphBuilder.finish().askView(taskId)

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

    public func execute(): Unit {
        checkFinalized()

        let graph = taskGraphBuilder.finish()
        let dirtyGraph = graph.filterGraph { it: TaskId => dirtyTasks.contains(it) }
        
        rebuild(taskStorage, dirtyGraph, graph, cache, jobs)
        cache.storeCache()

        finalized = true
    }

    public func executeClean(): Unit {
        checkFinalized()

        let graph = taskGraphBuilder.finish()
        
        rebuild(taskStorage, graph, graph, cache, jobs)
        cache.storeCache()

        finalized = true
    }

}

extend IncrementalEngine {
    protected func clearCache() {
        cache.clear()
    }
}