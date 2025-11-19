package cjpm.engine.graph

import std.collection.*

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

public class TaskGraph {
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
