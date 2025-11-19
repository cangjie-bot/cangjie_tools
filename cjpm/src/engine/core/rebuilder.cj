package cjpm.engine.core

import std.collection.*
import std.deriving.Derive
import std.sync.Semaphore

import cjpm.engine.cache.*
import cjpm.engine.graph.*

@Derive[Equatable]
private enum TaskResult {
    | Rebuild
    | Cached
    | Fatal
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

public func rebuild(taskStorage: HashMap<TaskId, ErasedTask>, dirtyGraph: TaskGraph, taskGraph: TaskGraph, cache: IncrementalCache, jobs: Int64): Unit {
    let semaphore = Semaphore(jobs)
    let futureMap = HashMap<TaskId, Future<TaskResult>>()

    for (id in dirtyGraph.toposort) {
        let future = spawn { =>
            semaphore.acquire()

            let dependenciesResult = dirtyGraph.incoming(id)
                .map { it => futureMap[it].get() }

            if (dependenciesResult.fatal) {
                semaphore.release()
                return Fatal
            }

            let dependencies = taskGraph.incoming(id)
            // Early cutoff
            try {
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
                        semaphore.release()
                        return Cached
                    }
                }

                let task = taskStorage[id]
                let newInputCache = dependencies |> filterMap { it: TaskId => cache.readOutputHash(it) } |> unorderedHash
                task.cleanCompute()
                if (let Some(hash) <- cache.readOutputHash(id)) {
                    if (task.computedHash() == hash) {
                        // Early cutoff
                        cache.update(id, newInputCache, task.computedHash(), task.asDataModel())
                        semaphore.release()
                        return Cached
                    } else {
                        cache.update(id, newInputCache, task.computedHash(), task.asDataModel())
                        semaphore.release()
                        return Rebuild
                    }
                } else {
                    cache.update(id, newInputCache, task.computedHash(), task.asDataModel())
                    semaphore.release()
                    return Rebuild
                }
            } catch (_: IncrementalEngineException) {
                semaphore.release()
                return Fatal
            }
        }
        futureMap[id] = future
    }

    for (future in futureMap.values()) {
        let _ = future.get()
    }
}
