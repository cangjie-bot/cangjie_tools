package cjpm.engine.tests.simple_test

import cjpm.engine.*
import std.fs.*
import std.unittest.*
import std.unittest.testmacro.*

@Test
class SimpleTest {
    let incremental = IncrementalEngine(
        cacheLocation: Path("./src/tests/simple_test/cache.json")
    )

    @BeforeAll
    func clearCache() {
        incremental.clearCache()
    }

    @TestCase
    func simple() {
        let task0 = incremental.task("task0") { => 
            let result = "Hello"
            result
        }

        let task1 = incremental.task("task1", task0) { s =>
            "${s}, World"
        }

        let mainTask = incremental.task("task2", task1) { s =>
            "${s}!"
        }

        incremental.ask("task2"); incremental.execute()

        @Assert(mainTask(), "Hello, World!")
    }
}
