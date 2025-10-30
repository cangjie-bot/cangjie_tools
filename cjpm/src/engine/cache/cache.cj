package cjpm.engine.cache

import std.fs.*
import std.collection.*

import stdx.encoding.json.*
import stdx.serialization.serialization.*

private class CacheItem <: Serializable<CacheItem> {
    private static let VALUE = "value"
    private static let INPUT_HASH = "inputHash"
    private static let OUTPUT_HASH = "outputHash"

    CacheItem(
        let value: DataModel,
        let inputHash: Hash,
        let outputHash: Hash
    ) { }

    public func serialize(): DataModel {
        DataModelStruct()
            .add(Field(VALUE, value))
            .add(field(INPUT_HASH, inputHash))
            .add(field(OUTPUT_HASH, outputHash))
    }

    public static func deserialize(dm: DataModel): CacheItem {
        let dms = match (dm) {
            case dms: DataModelStruct => dms
            case _ => throw DataModelException("this data is not DataModelStruct") // TODO: improve error message
        }
        let value = dms.get(VALUE)
        let inputHash = Hash.deserialize(dms.get(INPUT_HASH))
        let outputHash = Hash.deserialize(dms.get(OUTPUT_HASH))
        CacheItem(value, inputHash, outputHash)
    }
}

public class IncrementalCache {
    private let cache: HashMap<TaskId, CacheItem>

    public IncrementalCache(
        private let cacheLocation: Path,
    ) { 
        this.cache = readCacheFromJson(cacheLocation)
    }

    public func update(id: TaskId, inputHash: Hash, outputHash: Hash, dm: DataModel): Unit {
        cache.add(id, CacheItem(dm, inputHash, outputHash))
    }

    public func readValue(id: TaskId): ?DataModel {
        cache.get(id)?.value
    }

    public func readInputHash(id: TaskId): ?Hash {
        cache.get(id)?.inputHash
    }

    public func readOutputHash(id: TaskId): ?Hash {
        cache.get(id)?.outputHash
    }

    private static func readCacheFromJson(cacheLocation: Path): HashMap<TaskId, CacheItem> {
        let dm = readJsonFileOrEmpty(cacheLocation)
        try {
            HashMap<TaskId, CacheItem>.deserialize(dm)
        } catch (e: Exception) {
            HashMap()
        }
    }

    public func storeCache(): Unit {
        writeJsonToFile(cacheLocation, cache)
    }

    public func clear(): Unit {
        cache.clear()
        storeCache()
    }

}

private func readJsonFileOrEmpty(path: Path): DataModel {
    try {
        let barray = File.readFrom(path)
        let s = String.fromUtf8(barray)
        let json = JsonValue.fromStr(s)
        DataModel.fromJson(json)
    } catch (_: Exception) {
        DataModel.fromJson(JsonObject())
    }
}

private func writeJsonToFile(path: Path, cache: HashMap<TaskId, CacheItem>): Unit {
    try {
        let dm = cache.serialize()
        let json = dm.toJson()
        let s = json.toJsonString()
        let barray = s.toArray()
        File.writeTo(path, barray)
    } catch (_: Exception) { }
}
