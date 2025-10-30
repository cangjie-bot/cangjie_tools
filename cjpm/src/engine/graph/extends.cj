package cjpm.engine.graph

import std.collection.*

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
