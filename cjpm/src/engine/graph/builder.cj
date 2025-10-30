package cjpm.engine.graph

import std.collection.*

public class TaskGraphBuilder {
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
        let newGraph = graph |> 
            map { it: (TaskId, (Array<TaskId>, ArrayList<TaskId>)) => (it[0], TaskNode(it[1][0], it[1][1].toArray())) } |> 
            collectHashMap
        TaskGraph(newGraph, rootTasks.clone(), toposort.toArray())
    }
}
