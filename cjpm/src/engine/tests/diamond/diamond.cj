package cjpm.engine.tests.diamond

import cjpm.engine.*
import std.fs.*
import std.unittest.*
import std.unittest.testmacro.*

@Test
class Diamond {
    var incremental = initializeEngine()

    private static func initializeEngine(): IncrementalEngine {
        IncrementalEngine(
            cacheLocation: Path("./src/tests/diamond/cache.json")
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
    func hello() {
        let task0 = incremental.task("task0") { => 
            "Hello"
        }
        let task1 = incremental.task("task1", task0) { s =>
            "${s}, World"
        }
        let task2 = incremental.task("task2", task0) { s =>
            "${s}, Fellas"
        }
        let task3 = incremental.task("task3", task1, task2) { s1, s2 =>
            "s1: ${s1}; s2: ${s2}"
        }

        incremental.ask("task3"); incremental.execute()

        @Assert(task3(), "s1: Hello, World; s2: Hello, Fellas")
    }

    @TestCase
    func bye() {
        let task0 = incremental.task("task0") { => 
            "Bye"
        }
        let task1 = incremental.task("task1", task0) { s =>
            "${s}, World"
        }
        let task2 = incremental.task("task2", task0) { s =>
            "${s}, Fellas"
        }
        let task3 = incremental.task("task3", task1, task2) { s1, s2 =>
            "s1: ${s1}; s2: ${s2}"
        }

        incremental.ask("task3"); incremental.execute()

        @Assert(task3(), "s1: Bye, World; s2: Bye, Fellas")
    }

    @TestCase
    func partial() {
        { => 
            let task0 = incremental.task("task0") { => 
                "Test"
            }
            let task2 = incremental.task("task2", task0) { s =>
                "${s}, Fellas"
            }

            incremental.ask("task2"); incremental.execute()
            @Assert(task2(), "Test, Fellas")
        }()

        incremental = initializeEngine()

        { => 
            let task0 = incremental.task("task0") { => 
                "Best"
            }
            let task1 = incremental.task("task1", task0) { s =>
                "${s}, World"
            }

            incremental.ask("task1"); incremental.execute()
            @Assert(task1(), "Best, World")
        }()

        incremental = initializeEngine()

        { =>
            let task0 = incremental.task("task0") { => 
                "Wohoo"
            }
            let task1 = incremental.task("task1", task0) { s =>
                "${s}, World"
            }
            let task2 = incremental.task("task2", task0) { s =>
                "${s}, Fellas"
            }
            let task3 = incremental.task("task3", task1, task2) { s1, s2 =>
                "s1: ${s1}; s2: ${s2}"
            }

            incremental.ask("task3"); incremental.execute()

            @Assert(task3(), "s1: Wohoo, World; s2: Wohoo, Fellas")
        }()


    }

    @TestCase
    func afterPartial() {
        let task0 = incremental.task("task0") { => 
            "Test"
        }
        let _ = incremental.task("task1", task0) { s =>
            "${s}, World"
        }
        let task2 = incremental.task("task2", task0) { s =>
            "${s}, Fellas"
        }

        incremental.ask("task2"); incremental.execute()

        @Assert(task2(), "Test, Fellas")
    }
}
