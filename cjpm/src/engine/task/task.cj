package cjpm.engine.task

import cjpm.engine.cache.*

import stdx.serialization.serialization.*

sealed abstract class ErasedTask {
    protected var _value: Option<IncData> = None
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
            } catch(_: Exception) { }
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
