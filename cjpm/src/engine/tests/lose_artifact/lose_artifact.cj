package cjpm.engine.tests.lose_artifact

import cjpm.engine.*
import std.fs.*
import std.collection.*
import std.unittest.*
import std.unittest.testmacro.*

@Test
class LoseArtifact {
    var incremental = initializeEngine()

    private static func initializeEngine(): IncrementalEngine {
        IncrementalEngine(
            cacheLocation: Path("./src/tests/lose_artifact/cache.json")
        )
    }

    private static func artifactPath(name: String): PathWrapper {
        PathWrapper(Path("./src/tests/lose_artifact").join(name))
    }

    private static func createArtifact(pathwrap: PathWrapper, message: String): Unit {
        if (!exists(pathwrap.path)) {
            File.create(pathwrap.path)
            File.appendTo(pathwrap.path, "${message}\n".toArray())
        }
    }

    private static func loseArtifact(pathwrap: PathWrapper): Unit {
        if (exists(pathwrap.path)) {
            remove(pathwrap.path)
        }
    }

    private static func touchArtifact(pathwrap: PathWrapper): Unit {
        File.appendTo(pathwrap.path, "touched\n".toArray())
    }

    public static func readArtifact(pathwrap: PathWrapper): String {
        String.fromUtf8(File.readFrom(pathwrap.path))
    }

    private static let artifacts = HashSet<String>(["artifact1", "artifact2", "artifact3"])

    @BeforeAll
    func clearCache() {
        incremental.clearCache()
        for (artifact in artifacts) {
            loseArtifact(artifactPath(artifact))
        }
    }

    @BeforeEach
    public func newBuild(): Unit {
        incremental = initializeEngine()
    }

    @TestCase
    func createAndRead() {
        // Because this artifact is primary input, it's creation cannot be a part of task
        // It should be always present
        let pathwrap1 = artifactPath("artifact1")
        createArtifact(pathwrap1, "artifact1Content")

        // Here we will provide it as input and check it's existence
        let artifact1 = incremental.task("artifact1") { => 
            pathwrap1
        }

        let readArtifact = incremental.task("readArtifact", artifact1) { pw => 
            let value = readArtifact(pw)
            println("The new content of artifact is: ${value}")
            "${value} + reading"
        }
        incremental.ask(readArtifact.id); incremental.execute()
        @Assert(readArtifact(), "artifact1Content\n + reading")
    }

    @TestCase
    func createFromInputAndRead() {
        let task0 = incremental.task("artifact2Path") { => 
            "artifact2"
        }

        let task1 = incremental.task("createArtifact2", task0) { s =>
            let pathwrap = artifactPath(s)
            createArtifact(pathwrap, "artifact2Content")
            pathwrap
        }

        let readArtifact = incremental.task("readArtifact2", task1) { pw =>
            let value = readArtifact(pw)
            println("The new content of artifact is: ${value}")
            "${value} + reading"
        }

        incremental.ask("readArtifact2"); incremental.execute()
        @Assert(readArtifact(), "artifact2Content\n + reading")
    }

    @TestCase
    func modifyArtifact() {
        sleep(Duration.second * 2)
        touchArtifact(artifactPath("artifact2"))

        let task0 = incremental.task("artifact2Path") { => 
            "artifact2"
        }

        let task1 = incremental.task("createArtifact2", task0) { s =>
            let pathwrap = artifactPath(s)
            createArtifact(pathwrap, "artifact2Content")
            pathwrap
        }

        let readArtifact = incremental.task("readArtifact2", task1) { pw =>
            let value = readArtifact(pw)
            println("The new content of artifact is: ${value}")
            "${value} + reading"
        }

        incremental.ask("readArtifact2"); incremental.execute()
        @Assert(readArtifact(), "artifact2Content\ntouched\n + reading")
    }

    @TestCase
    func loseArtifactAndRecreate() {
        loseArtifact(artifactPath("artifact2"))
        sleep(Duration.second * 2)

        let task0 = incremental.task("artifact2Path") { => 
            "artifact2"
        }

        let task1 = incremental.task("createArtifact2", task0) { s =>
            let pathwrap = artifactPath(s)
            createArtifact(pathwrap, "artifact2Content")
            pathwrap
        }

        let readArtifact = incremental.task("readArtifact2", task1) { pw =>
            let value = readArtifact(pw)
            println("The new content of artifact is: ${value}")
            "${value} + reading"
        }

        incremental.ask("readArtifact2"); incremental.execute()
        @Assert(readArtifact(), "artifact2Content\n + reading")
    }
}
