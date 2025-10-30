package cjpm.engine.types

import stdx.serialization.serialization.*

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
 * Cachable data
 */
public interface SerializableIncData<T> <: IncData where T <: Serializable<T> { 
    /**
     * case None  - Although, value is known, the current state is invalid and the task should be recomputed
     * case value - The value is valid, and can be reused
     */
    func incHash(): ?Hash
}

/**
 * TODO: Description
 */

public struct TaskUnit { }
extend TaskUnit <: Serializable<TaskUnit> & SerializableIncData<TaskUnit> {
    public func serialize(): DataModel { DataModelNull() }
    public static func deserialize(_: DataModel): TaskUnit { TaskUnit() }
    public func incHash(): ?Hash { 0 }
}
