package cjpm.engine.types

import std.fs.*
import std.collection.*

import stdx.serialization.serialization.*

public struct PackageSources <: Serializable<PackageSources> {
    PackageSources(
        // TODO: multiplatform
        public let dirpath: Path
    ) { }

    public func serialize(): DataModel {
        dirpath.toString().serialize()
    }

    public static func deserialize(dm: DataModel): PackageSources {
        let dmstr = dm as DataModelString ?? throw DataModelException("this data is not DataModelString")
        PackageSources(Path(dmstr.getValue()))
    }
}

extend PackageSources <: SerializableIncData<PackageSources> {
    public func incHash(): ?Hash { 
        if (exists(dirpath) && FileInfo(canonicalize(dirpath)).isDirectory()) {
            Directory.readFrom(dirpath) |> 
                filter { it => it.isRegular() } |>
                map { it => it.lastModificationTime } |>
                unorderedHash
        }
        return None 
    }
}

/**
 * Use it, when you don't care about the value
 * 
 * TODO: replace with normal struct
 */
public struct PackageCompileTimestamp <: Serializable<PackageCompileTimestamp> {
    PackageCompileTimestamp(
        public let timestamp: Int64
    ) { }


    public func serialize(): DataModel {
        timestamp.serialize()
    }

    public static func deserialize(dm: DataModel): PackageCompileTimestamp {
        let dmint = dm as DataModelInt ?? throw DataModelException("this data is not DataModelInt")
        PackageCompileTimestamp(dmint.getValue())
    }
}

extend PackageCompileTimestamp <: SerializableIncData<PackageCompileTimestamp> {
    public func incHash(): ?Hash { 
        timestamp
    }
}
