package cjpm.engine.types

import stdx.serialization.serialization.*
import std.fs.*

extend String <: SerializableIncData<String> {
    public func incHash(): ?Hash {
        this.hashCode()
    }
}

extend Int64 <: SerializableIncData<Int64> {
    public func incHash(): ?Hash {
        this.hashCode()
    }
}

extend <T> Array<T> <: SerializableIncData<Array<T>> where T <: SerializableIncData<T> & ToString {
    public func incHash(): ?Hash {
        var hasher = DefaultHasher()
        for (item in this) {
            hasher.write(item.incHash() ?? return None)
        }
        hasher.finish()
    }
}

public struct PathWrapper <: Serializable<PathWrapper> & ToString {
    public PathWrapper(
        public let path: Path
    ) { }

    public func serialize(): DataModel {
        path.toString().serialize()
    }

    public static func deserialize(dm: DataModel): PathWrapper {
        let dmstr = dm as DataModelString ?? throw DataModelException()
        PathWrapper(Path(dmstr.getValue()))
    } 

    public func toString(): String {
        path.toString()
    }
}

extend PathWrapper <: SerializableIncData<PathWrapper> {
    public func incHash(): ?Hash {
        if (exists(this.path)) {
            FileInfo(this.path).lastModificationTime.hashCode()
        } else { None }
    }
}