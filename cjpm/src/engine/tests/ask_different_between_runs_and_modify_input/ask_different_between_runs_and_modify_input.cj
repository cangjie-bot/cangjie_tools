package cjpm.engine.tests.ask_different_between_runs_and_modify_input

import cjpm.engine.*
import std.fs.*
import std.unittest.*
import std.unittest.testmacro.*

@Test
class AskDifferentBetweenRuns {
    var incremental = initializeEngine()

    private static func initializeEngine(): IncrementalEngine {
        IncrementalEngine(
            cacheLocation: Path("./src/tests/ask_different_between_runs_and_modify_input/cache.json")
        )
    }

    @BeforeAll
    func clearCache() {
        incremental.clearCache()
    }

    @BeforeEach
    public func newBuild(): Unit {
        incremental = initializeEngine()
    }

    @TestCase
    func askTask1() {
        let task0 = incremental.task("task0") { => 
            "Hello"
        }
        let task1 = incremental.task("task1", task0) { s =>
            "${s}, World"
        }
        let _ = incremental.task("task2", task1) { s =>
            "${s}!"
        }
        incremental.ask("task1"); incremental.execute()

        @Assert(task1(), "Hello, World")
    }

    @TestCase
    func askTask2() {
        let task0 = incremental.task("task0") { => 
            "Bye"
        }
        let task1 = incremental.task("task1", task0) { s =>
            "${s}, World"
        }
        let mainTask = incremental.task("task2", task1) { s =>
            "${s}!"
        }
        incremental.ask("task2"); incremental.execute()

        @Assert(mainTask(), "Bye, World!")
    }
}
