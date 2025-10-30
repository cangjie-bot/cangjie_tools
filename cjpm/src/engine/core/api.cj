package cjpm.engine.core

extend IncrementalEngine {
    public func task<R>(id: TaskId, valueComp: () -> R): Task<R> 
        where 
            R <: SerializableIncData<R>
    {
        checkFinalized()

        taskGraphBuilder.addTask(id, dependencies: [])
        let out = Task(id, cache, valueComp)
        taskStorage.add(id, out)
        return out
    }

    public func task<T0, R>(id: TaskId, i1: Task<T0>, comp: (T0) -> R): Task<R> 
        where 
            T0 <: SerializableIncData<T0>,
            R  <: SerializableIncData<R>
    {
        checkFinalized()

        let ids: Array<TaskId> = [i1.id]
        taskGraphBuilder.addTask(id, dependencies: ids)
        let out = Task(id, cache, { => comp(i1())})
        taskStorage.add(id, out)
        return out
    }

    public func task<T0, T1, R>(id: TaskId, i1: Task<T0>, i2: Task<T1>, comp: (T0, T1) -> R): Task<R> 
        where 
            T0 <: SerializableIncData<T0>,
            T1 <: SerializableIncData<T1>,
            R  <: SerializableIncData<R>
    {
        checkFinalized()

        let ids: Array<TaskId> = [i1.id, i2.id]
        taskGraphBuilder.addTask(id, dependencies: ids)
        let out = Task(id, cache, { => comp(i1(), i2())})
        taskStorage.add(id, out)
        return out
    }
}
