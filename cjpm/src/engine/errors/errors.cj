package cjpm.engine.errors

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