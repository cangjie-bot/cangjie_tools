package cjpm.engine.util

import std.sort.sort
import std.collection.{map, collectArray}

private type Hash = Int64

public func unorderedHash(iter: Iterable<Hash>): Hash {
    let hashes = iter |> collectArray
    unorderedHashHelper(hashes)
}

public func unorderedHash<T>(iter: Iterable<T>): Hash where T <: Hashable {
    let hashes = iter |> map { it => it.hashCode() } |> collectArray
    unorderedHashHelper(hashes)
}

private func unorderedHashHelper(hashes: Array<Hash>): Hash {
    sort(hashes)
    var hasher = DefaultHasher() 
    hasher.write(hashes.size)
    for (hash in hashes) {
        hasher.write(hash)
    }
    hasher.finish()
}
