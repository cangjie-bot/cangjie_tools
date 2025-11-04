// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "IndexStorage.cpp"
#include <string>
#include <vector>
#include <unordered_map>

using namespace ark::lsp;

// Test SplitFileName function
TEST(IndexStorageTest, SplitFileName_ValidFile) {
    std::string file = "test.package.hash.ast";
    auto result = SplitFileName(file);
    EXPECT_EQ(result.first, "test.package");
    EXPECT_EQ(result.second, "hash");
}

TEST(IndexStorageTest, SplitFileName_NoExtension) {
    std::string file = "testpackagehashast";
    auto result = SplitFileName(file);
    EXPECT_TRUE(result.first.empty());
    EXPECT_TRUE(result.second.empty());
}

TEST(IndexStorageTest, SplitFileName_SingleDot) {
    std::string file = "test.package.ast";
    auto result = SplitFileName(file);
    EXPECT_EQ(result.first, "test");
    EXPECT_EQ(result.second, "package");
}

// Test MergeFileName function
TEST(IndexStorageTest, MergeFileName) {
    std::string fullPkgName = "test.package";
    std::string hashCode = "abc123";
    std::string extension = "idx";

    std::string result = MergeFileName(fullPkgName, hashCode, extension);
    EXPECT_EQ(result, "test.package.abc123.idx");
}

// Test CacheManager::IsStale function
TEST(IndexStorageTest, CacheManagerIsStale_NoCache) {
    CacheManager cacheManager("test_path");
    bool result = cacheManager.IsStale("nonexistent.package", "hash123");
    EXPECT_TRUE(result);
}

// Test CacheManager::GetShardPathFromFilePath function
TEST(IndexStorageTest, CacheManagerGetShardPathFromFilePath) {
    CacheManager cacheManager("test_path");
    std::string result = cacheManager.GetShardPathFromFilePath("test/package", "hash123");
    // Note: The actual path depends on the value of indexDir, here we just check if it contains key parts
    EXPECT_NE(result.find("test.package"), std::string::npos);
    EXPECT_NE(result.find("hash123.idx"), std::string::npos);
}

// Test ReadSymbolLocation function
TEST(IndexStorageTest, ReadSymbolLocation_ValidData) {
    Symbol symbol;
    // Since IdxFormat::Symbol is a FlatBuffer generated class, we cannot construct it directly
    // This test is mainly to ensure the function can be called without crashing
    EXPECT_NO_THROW(ReadSymbolLocation(symbol, nullptr));
}

// Test ReadSymbolMacro function
TEST(IndexStorageTest, ReadSymbolMacro_ValidData) {
    Symbol symbol;
    // Since IdxFormat::Symbol is a FlatBuffer generated class, we cannot construct it directly
    // This test is mainly to ensure the function can be called without crashing
    EXPECT_NO_THROW(ReadSymbolMacro(symbol, nullptr));
}

// Test ReadSymbol function
TEST(IndexStorageTest, ReadSymbol_ValidData) {
    Symbol symbol;
    // Since IdxFormat::Symbol is a FlatBuffer generated class, we cannot construct it directly
    // This test is mainly to ensure the function can be called without crashing
    EXPECT_NO_THROW(ReadSymbol(symbol, nullptr));
}

// Test ReadRefLocation function
TEST(IndexStorageTest, ReadRefLocation_ValidData) {
    Ref ref;
    // Since IdxFormat::Ref is a FlatBuffer generated class, we cannot construct it directly
    // This test is mainly to ensure the function can be called without crashing
    EXPECT_NO_THROW(ReadRefLocation(ref, nullptr));
}

// Test ReadRef function
TEST(IndexStorageTest, ReadRef_ValidData) {
    Ref ref;
    // Since IdxFormat::Ref is a FlatBuffer generated class, we cannot construct it directly
    // This test is mainly to ensure the function can be called without crashing
    EXPECT_NO_THROW(ReadRef(ref, nullptr));
}

// Test ReadRelation function
TEST(IndexStorageTest, ReadRelation_ValidData) {
    Relation relation;
    // Since IdxFormat::Relation is a FlatBuffer generated class, we cannot construct it directly
    // This test is mainly to ensure the function can be called without crashing
    EXPECT_NO_THROW(ReadRelation(relation, nullptr));
}

// Test ReadCrossSymbolLocation function
TEST(IndexStorageTest, ReadCrossSymbolLocation_ValidData) {
    CrossSymbol crossSymbol;
    // Since IdxFormat::CrossSymbol is a FlatBuffer generated class, we cannot construct it directly
    // This test is mainly to ensure the function can be called without crashing
    EXPECT_NO_THROW(ReadCrossSymbolLocation(crossSymbol, nullptr));
}

// Test ReadCrossSymbol function
TEST(IndexStorageTest, ReadCrossSymbol_ValidData) {
    CrossSymbol crossSymbol;
    // Since IdxFormat::CrossSymbol is a FlatBuffer generated class, we cannot construct it directly
    // This test is mainly to ensure the function can be called without crashing
    EXPECT_NO_THROW(ReadCrossSymbol(crossSymbol, nullptr));
}

// Test StoreSymbol function
TEST(IndexStorageTest, StoreSymbol_ValidData) {
    flatbuffers::FlatBufferBuilder builder;
    Symbol symbol;
    symbol.id = 1;
    symbol.name = "testSymbol";
    symbol.scope = "testScope";

    // This test is mainly to ensure the function can be called without crashing
    EXPECT_NO_THROW(StoreSymbol(builder, symbol));
}

// Test StoreRef function
TEST(IndexStorageTest, StoreRef_ValidData) {
    flatbuffers::FlatBufferBuilder builder;
    Ref ref;

    // This test is mainly to ensure the function can be called without crashing
    EXPECT_NO_THROW(StoreRef(builder, ref));
}

// Test StoreExtend function
TEST(IndexStorageTest, StoreExtend_ValidData) {
    flatbuffers::FlatBufferBuilder builder;
    ExtendItem extendItem;
    extendItem.id = 1;
    extendItem.modifier = Modifier::PUBLIC;
    extendItem.interfaceName = "TestInterface";

    // This test is mainly to ensure the function can be called without crashing
    EXPECT_NO_THROW(StoreExtend(builder, extendItem));
}

// Test StoreRelation function
TEST(IndexStorageTest, StoreRelation_ValidData) {
    flatbuffers::FlatBufferBuilder builder;
    Relation relation;
    relation.subject = 1;
    relation.predicate = RelationKind::EXTEND;
    relation.object = 2;

    // This test is mainly to ensure the function can be called without crashing
    EXPECT_NO_THROW(StoreRelation(builder, relation));
}

// Test StoreCrossSymbol function
TEST(IndexStorageTest, StoreCrossSymbol_ValidData) {
    flatbuffers::FlatBufferBuilder builder;
    CrossSymbol crossSymbol;
    crossSymbol.id = 1;
    crossSymbol.name = "testCrossSymbol";
    crossSymbol.crossType = CrossType::UNDEFINED;
    crossSymbol.container = 2;
    crossSymbol.containerName = "testContainer";

    // This test is mainly to ensure the function can be called without crashing
    EXPECT_NO_THROW(StoreCrossSymbol(builder, crossSymbol));
}

// Test AstFileHandler::LoadShard function
TEST(IndexStorageTest, AstFileHandlerLoadShard_NonExistentFile) {
    AstFileHandler handler;
    auto result = handler.LoadShard("nonexistent_file.ast");
    EXPECT_FALSE(result.has_value());
}

// Test AstFileHandler::StoreShard function
TEST(IndexStorageTest, AstFileHandlerStoreShard_NullOut) {
    AstFileHandler handler;
    EXPECT_NO_THROW(handler.StoreShard("test_file.ast", nullptr));
}

// Test CacheManager::InitDir function
TEST(IndexStorageTest, CacheManagerInitDir) {
    // This test requires an actual path, but we cannot guarantee the path exists or is writable
    // Mainly test whether the function can be called without crashing
    EXPECT_NO_THROW(CacheManager cacheManager("test_path"));
}

// Test CacheManager::UpdateIdMap function
TEST(IndexStorageTest, CacheManagerUpdateIdMap) {
    CacheManager cacheManager("test_path");
    EXPECT_NO_THROW(cacheManager.UpdateIdMap("test.package", "hash123"));
}

// Test CacheManager::Load function
TEST(IndexStorageTest, CacheManagerLoad_NonExistentPackage) {
    CacheManager cacheManager("test_path");
    auto result = cacheManager.Load("nonexistent.package");
    EXPECT_FALSE(result.has_value());
}

// Test CacheManager::Store function
TEST(IndexStorageTest, CacheManagerStore_EmptyDigest) {
    CacheManager cacheManager("test_path");
    std::vector<uint8_t> buffer = {1, 2, 3, 4};
    // Empty digest should cause the function to return directly
    EXPECT_NO_THROW(cacheManager.Store("test.package", "", buffer));
}
