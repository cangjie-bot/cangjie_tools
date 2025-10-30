package cjpm.engine.tests.input_task_modification

import cjpm.engine.*
import std.fs.*
import std.unittest.*
import std.unittest.testmacro.*

@Test
class InputTaskModification {
    var incremental = initializeEngine()

    private static func initializeEngine(): IncrementalEngine {
        IncrementalEngine(
            cacheLocation: Path("./src/tests/input_task_modification/cache.json")
        )
    }

    @BeforeAll
    public func clearCache(): Unit {
        incremental.clearCache()
    }

    @BeforeEach
    public func newBuild(): Unit {
        incremental = initializeEngine()
    }

    private func buildChain(value: String): Task<String> {
        let task0 = incremental.task("task0") { => 
            value
        }

        let task1 = incremental.task("task1", task0) { s =>
            "${s}, World"
        }

        let mainTask = incremental.task("task2", task1) { s =>
            "${s}!"
        }

        mainTask
    }

    @TestCase
    func helloworld() {
        let mainTask = buildChain("Hello")
        incremental.ask("task2"); incremental.execute()
        @Assert(mainTask(), "Hello, World!")
    }

    @TestCase
    func byeworld() {
        let mainTask = buildChain("Bye")
        incremental.ask("task2"); incremental.execute()
        @Assert(mainTask(), "Bye, World!")
    }
}
