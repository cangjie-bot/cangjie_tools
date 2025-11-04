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
    flatbuffers::FlatBufferBuilder builder;

    // Create location data
    IdxFormat::Position startPos(1, 1, 1);
    IdxFormat::Position endPos(1, 1, 10);
    auto fileUri = builder.CreateString("file:///test.cj");
    auto location = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);

    // Create symbol with location
    auto name = builder.CreateString("testSymbol");
    auto scope = builder.CreateString("testScope");
    auto symbol = IdxFormat::CreateSymbol(builder, 1, name, scope, location);

    // Create package with symbol
    std::vector<flatbuffers::Offset<IdxFormat::Symbol>> symbols = {symbol};
    auto symbolSlab = builder.CreateVector(symbols);
    auto package = IdxFormat::CreateHashedPackage(builder, symbolSlab);
    builder.Finish(package);

    // Get the actual symbol from the buffer
    auto pkg = IdxFormat::GetHashedPackage(builder.GetBufferPointer());
    auto sym = pkg->symbol_slab()->Get(0);

    Symbol outSymbol;
    EXPECT_NO_THROW(ReadSymbolLocation(outSymbol, sym));
}

// Test ReadSymbolMacro function
TEST(IndexStorageTest, ReadSymbolMacro_ValidData) {
    flatbuffers::FlatBufferBuilder builder;

    // Create macro call location
    IdxFormat::Position macroStart(2, 1, 1);
    IdxFormat::Position macroEnd(2, 1, 5);
    auto macroFileUri = builder.CreateString("file:///macro.cj");
    auto macroLocation = IdxFormat::CreateLocation(builder, &macroStart, &macroEnd, macroFileUri);

    // Create main location
    IdxFormat::Position startPos(1, 1, 1);
    IdxFormat::Position endPos(1, 1, 10);
    auto fileUri = builder.CreateString("file:///test.cj");
    auto location = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);

    // Create declaration location (cannot be nullptr)
    auto declLocation = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);

    // Create symbol with macro call
    auto name = builder.CreateString("testSymbol");
    auto scope = builder.CreateString("testScope");
    auto signature = builder.CreateString("testSignature");
    auto returnType = builder.CreateString("Int");
    auto curModule = builder.CreateString("testCurModule");
    auto symbol = IdxFormat::CreateSymbol(builder, 1, name, scope, location, declLocation, 0,
                                          signature, returnType, false, 0, false, false,
                                          curModule, macroLocation);

    // Create package with symbol
    std::vector<flatbuffers::Offset<IdxFormat::Symbol>> symbols = {symbol};
    auto symbolSlab = builder.CreateVector(symbols);
    auto package = IdxFormat::CreateHashedPackage(builder, symbolSlab);
    builder.Finish(package);

    // Get the actual symbol from the buffer
    auto pkg = IdxFormat::GetHashedPackage(builder.GetBufferPointer());
    auto sym = pkg->symbol_slab()->Get(0);

    Symbol outSymbol;
    EXPECT_NO_THROW(ReadSymbolMacro(outSymbol, sym));
}


// Test ReadSymbol function
TEST(IndexStorageTest, ReadSymbol_ValidData) {
    flatbuffers::FlatBufferBuilder builder;

    // Create location data
    IdxFormat::Position startPos(1, 1, 1);
    IdxFormat::Position endPos(1, 1, 10);
    auto fileUri = builder.CreateString("file:///test.cj");
    auto location = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);

    // Create symbol
    auto name = builder.CreateString("testSymbol");
    auto scope = builder.CreateString("testScope");
    auto signature = builder.CreateString("testSignature");
    auto returnType = builder.CreateString("Int");
    auto module = builder.CreateString("test.module");
    auto declaration = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);
    auto symbol = IdxFormat::CreateSymbol(builder, 1, name, scope, location, declaration,
                                          static_cast<uint16_t>(AST::ASTKind::FUNC_TYPE),
                                          signature, returnType, false,
                                          static_cast<uint8_t>(Modifier::PUBLIC),
                                          false, false, module);

    // Create package with symbol
    std::vector<flatbuffers::Offset<IdxFormat::Symbol>> symbols = {symbol};
    auto symbolSlab = builder.CreateVector(symbols);
    auto package = IdxFormat::CreateHashedPackage(builder, symbolSlab);
    builder.Finish(package);

    // Get the actual symbol from the buffer
    auto pkg = IdxFormat::GetHashedPackage(builder.GetBufferPointer());
    auto sym = pkg->symbol_slab()->Get(0);

    Symbol outSymbol;
    EXPECT_NO_THROW(ReadSymbol(outSymbol, sym));
}

// Test ReadRefLocation function
TEST(IndexStorageTest, ReadRefLocation_ValidData) {
    flatbuffers::FlatBufferBuilder builder;

    // Create ref location
    IdxFormat::Position startPos(1, 1, 1);
    IdxFormat::Position endPos(1, 1, 10);
    auto fileUri = builder.CreateString("file:///test.cj");
    auto location = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);

    // Create ref
    auto ref = IdxFormat::CreateRef(builder, location, static_cast<uint16_t>(RefKind::REFERENCE), 1);

    // Create Sym2Ref
    std::vector<flatbuffers::Offset<IdxFormat::Ref>> refs = {ref};
    auto refsVec = builder.CreateVector(refs);
    auto sym2Ref = IdxFormat::CreateSym2Ref(builder, 1, refsVec);

    // Create package with ref
    std::vector<flatbuffers::Offset<IdxFormat::Sym2Ref>> sym2Refs = {sym2Ref};
    auto refSlab = builder.CreateVector(sym2Refs);
    // Create symbol
    auto name = builder.CreateString("testSymbol");
    auto scope = builder.CreateString("testScope");
    auto signature = builder.CreateString("testSignature");
    auto returnType = builder.CreateString("Int");
    auto module = builder.CreateString("test.module");
    auto declaration = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);
    auto symbol = IdxFormat::CreateSymbol(builder, 1, name, scope, location, declaration,
                                          static_cast<uint16_t>(AST::ASTKind::FUNC_TYPE),
                                          signature, returnType, false,
                                          static_cast<uint8_t>(Modifier::PUBLIC),
                                          false, false, module);

    // Create package with symbol
    std::vector<flatbuffers::Offset<IdxFormat::Symbol>> symbols = {symbol};
    auto symbolSlab = builder.CreateVector(symbols);
    auto package = IdxFormat::CreateHashedPackage(builder, symbolSlab, refSlab);
    builder.Finish(package);

    // Get the actual ref from the buffer
    auto pkg = IdxFormat::GetHashedPackage(builder.GetBufferPointer());
    auto refEntry = pkg->ref_slab()->Get(0);
    auto actualRef = refEntry->refs()->Get(0);

    Ref outRef;
    EXPECT_NO_THROW(ReadRefLocation(outRef, actualRef));
}

// Test ReadRef function
TEST(IndexStorageTest, ReadRef_ValidData) {
    flatbuffers::FlatBufferBuilder builder;

    // Create ref location
    IdxFormat::Position startPos(1, 1, 1);
    IdxFormat::Position endPos(1, 1, 10);
    auto fileUri = builder.CreateString("file:///test.cj");
    auto location = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);

    // Create ref
    auto ref = IdxFormat::CreateRef(builder, location, static_cast<uint16_t>(RefKind::REFERENCE), 1, true);

    // Create Sym2Ref
    std::vector<flatbuffers::Offset<IdxFormat::Ref>> refs = {ref};
    auto refsVec = builder.CreateVector(refs);
    auto sym2Ref = IdxFormat::CreateSym2Ref(builder, 1, refsVec);

    // Create package with ref
    std::vector<flatbuffers::Offset<IdxFormat::Sym2Ref>> sym2Refs = {sym2Ref};
    auto refSlab = builder.CreateVector(sym2Refs);
    // Create symbol
    auto name = builder.CreateString("testSymbol");
    auto scope = builder.CreateString("testScope");
    auto signature = builder.CreateString("testSignature");
    auto returnType = builder.CreateString("Int");
    auto module = builder.CreateString("test.module");
    auto declaration = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);
    auto symbol = IdxFormat::CreateSymbol(builder, 1, name, scope, location, declaration,
                                          static_cast<uint16_t>(AST::ASTKind::FUNC_TYPE),
                                          signature, returnType, false,
                                          static_cast<uint8_t>(Modifier::PUBLIC),
                                          false, false, module);

    // Create package with symbol
    std::vector<flatbuffers::Offset<IdxFormat::Symbol>> symbols = {symbol};
    auto symbolSlab = builder.CreateVector(symbols);
    auto package = IdxFormat::CreateHashedPackage(builder, symbolSlab, refSlab);
    builder.Finish(package);

    // Get the actual ref from the buffer
    auto pkg = IdxFormat::GetHashedPackage(builder.GetBufferPointer());
    auto refEntry = pkg->ref_slab()->Get(0);
    auto actualRef = refEntry->refs()->Get(0);

    Ref outRef;
    EXPECT_NO_THROW(ReadRef(outRef, actualRef));
}

// Test ReadRelation function
TEST(IndexStorageTest, ReadRelation_ValidData) {
    flatbuffers::FlatBufferBuilder builder;


    // Create relation
    auto relation = IdxFormat::CreateRelation(builder, 1, static_cast<uint16_t>(RelationKind::EXTEND), 2);

    // Create ref location
    IdxFormat::Position startPos(1, 1, 1);
    IdxFormat::Position endPos(1, 1, 10);
    auto fileUri = builder.CreateString("file:///test.cj");
    auto location = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);

    // Create ref
    auto ref = IdxFormat::CreateRef(builder, location, static_cast<uint16_t>(RefKind::REFERENCE), 1);

    // Create package with relation
    std::vector<flatbuffers::Offset<IdxFormat::Relation>> relations = {relation};
    auto relationSlab = builder.CreateVector(relations);
    // Create symbol
    auto name = builder.CreateString("testSymbol");
    auto scope = builder.CreateString("testScope");
    auto signature = builder.CreateString("testSignature");
    auto returnType = builder.CreateString("Int");
    auto module = builder.CreateString("test.module");
    auto declaration = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);
    auto symbol = IdxFormat::CreateSymbol(builder, 1, name, scope, location, declaration,
                                          static_cast<uint16_t>(AST::ASTKind::FUNC_TYPE),
                                          signature, returnType, false,
                                          static_cast<uint8_t>(Modifier::PUBLIC),
                                          false, false, module);

    // Create package with symbol
    std::vector<flatbuffers::Offset<IdxFormat::Symbol>> symbols = {symbol};
    auto symbolSlab = builder.CreateVector(symbols);
    // Create Sym2Ref
    std::vector<flatbuffers::Offset<IdxFormat::Ref>> refs = {ref};
    auto refsVec = builder.CreateVector(refs);
    auto sym2Ref = IdxFormat::CreateSym2Ref(builder, 1, refsVec);
    // Create package with ref
    std::vector<flatbuffers::Offset<IdxFormat::Sym2Ref>> sym2Refs = {sym2Ref};
    auto refSlab = builder.CreateVector(sym2Refs);

    auto package = IdxFormat::CreateHashedPackage(builder, symbolSlab, refSlab, relationSlab);
    builder.Finish(package);

    // Get the actual relation from the buffer
    auto pkg = IdxFormat::GetHashedPackage(builder.GetBufferPointer());
    auto actualRelation = pkg->relation_slab()->Get(0);

    Relation outRelation;
    EXPECT_NO_THROW(ReadRelation(outRelation, actualRelation));
}

// Test ReadCrossSymbolLocation function
TEST(IndexStorageTest, ReadCrossSymbolLocation_ValidData) {
    flatbuffers::FlatBufferBuilder builder;

    // Create cross symbol location
    IdxFormat::Position startPos(1, 1, 1);
    IdxFormat::Position endPos(1, 1, 10);
    auto fileUri = builder.CreateString("file:///test.cj");
    auto location = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);

    // Create cross symbol
    auto name = builder.CreateString("testCrossSymbol");
    auto containerName = builder.CreateString("testContainer");
    auto crossSymbol = IdxFormat::CreateCrossSymbol(builder, 1, name,
                                                    static_cast<uint8_t>(CrossType::ARK_TS_WITH_INTEROP),
                                                    location, 2, containerName);

    // Create package with cross symbol
    std::vector<flatbuffers::Offset<IdxFormat::CrossSymbol>> crossSymbols = {crossSymbol};
    auto crossSymbolSlab = builder.CreateVector(crossSymbols);

    // Create relation
    auto relation = IdxFormat::CreateRelation(builder, 1, static_cast<uint16_t>(RelationKind::EXTEND), 2);

    // Create ref
    auto ref = IdxFormat::CreateRef(builder, location, static_cast<uint16_t>(RefKind::REFERENCE), 1);

    // Create package with relation
    std::vector<flatbuffers::Offset<IdxFormat::Relation>> relations = {relation};
    auto scope = builder.CreateString("testScope");
    auto signature = builder.CreateString("testSignature");
    auto returnType = builder.CreateString("Int");
    auto module = builder.CreateString("test.module");
    auto declaration = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);
    auto symbol = IdxFormat::CreateSymbol(builder, 1, name, scope, location, declaration,
                                          static_cast<uint16_t>(AST::ASTKind::FUNC_TYPE),
                                          signature, returnType, false,
                                          static_cast<uint8_t>(Modifier::PUBLIC),
                                          false, false, module);
    // Create package with symbol
    std::vector<flatbuffers::Offset<IdxFormat::Symbol>> symbols = {symbol};
    auto symbolSlab = builder.CreateVector(symbols);
    // Create Sym2Ref
    std::vector<flatbuffers::Offset<IdxFormat::Ref>> refs = {ref};
    auto refsVec = builder.CreateVector(refs);
    auto sym2Ref = IdxFormat::CreateSym2Ref(builder, 1, refsVec);
    // Create package with ref
    std::vector<flatbuffers::Offset<IdxFormat::Sym2Ref>> sym2Refs = {sym2Ref};
    auto refSlab = builder.CreateVector(sym2Refs);

    // Create package with relation
    auto relationSlab = builder.CreateVector(relations);

    // Create extend slab
    auto interfaceName = builder.CreateString("TestInterface");
    auto extend = IdxFormat::CreateExtend(builder, 1, static_cast<uint8_t>(Modifier::PUBLIC), interfaceName);

    std::vector<flatbuffers::Offset<IdxFormat::Extend>> extends = {extend};
    auto extendsVec = builder.CreateVector(extends);

    auto sym2Extend = IdxFormat::CreateSym2Extend(builder, 1, extendsVec);

    std::vector<flatbuffers::Offset<IdxFormat::Sym2Extend>> sym2Extends = {sym2Extend};
    auto extendSlab = builder.CreateVector(sym2Extends);

    auto package = IdxFormat::CreateHashedPackage(builder, symbolSlab, refSlab, relationSlab, extendSlab, crossSymbolSlab);
    builder.Finish(package);

    // Get the actual cross symbol from the buffer
    auto pkg = IdxFormat::GetHashedPackage(builder.GetBufferPointer());
    auto actualCrossSymbol = pkg->cross_symbol_slab()->Get(0);

    CrossSymbol outCrossSymbol;
    EXPECT_NO_THROW(ReadCrossSymbolLocation(outCrossSymbol, actualCrossSymbol));
}

// Test ReadCrossSymbol function
TEST(IndexStorageTest, ReadCrossSymbol_ValidData) {
    flatbuffers::FlatBufferBuilder builder;

    // Create cross symbol location
    IdxFormat::Position startPos(1, 1, 1);
    IdxFormat::Position endPos(1, 1, 10);
    auto fileUri = builder.CreateString("file:///test.cj");
    auto location = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);

    // Create cross symbol
    auto name = builder.CreateString("testCrossSymbol");
    auto containerName = builder.CreateString("testContainer");
    auto crossSymbol = IdxFormat::CreateCrossSymbol(builder, 1, name,
                                                    static_cast<uint8_t>(CrossType::ARK_TS_WITH_INTEROP),
                                                    location, 2, containerName);

    // Create package with cross symbol
    std::vector<flatbuffers::Offset<IdxFormat::CrossSymbol>> crossSymbols = {crossSymbol};
    auto crossSymbolSlab = builder.CreateVector(crossSymbols);
    // Create ref
    auto ref = IdxFormat::CreateRef(builder, location, static_cast<uint16_t>(RefKind::REFERENCE), 1);
    // Create relation
    auto relation = IdxFormat::CreateRelation(builder, 1, static_cast<uint16_t>(RelationKind::EXTEND), 2);

    // Create package with relation
    std::vector<flatbuffers::Offset<IdxFormat::Relation>> relations = {relation};
    auto scope = builder.CreateString("testScope");
    auto signature = builder.CreateString("testSignature");
    auto returnType = builder.CreateString("Int");
    auto module = builder.CreateString("test.module");
    auto declaration = IdxFormat::CreateLocation(builder, &startPos, &endPos, fileUri);
    auto symbol = IdxFormat::CreateSymbol(builder, 1, name, scope, location, declaration,
                                          static_cast<uint16_t>(AST::ASTKind::FUNC_TYPE),
                                          signature, returnType, false,
                                          static_cast<uint8_t>(Modifier::PUBLIC),
                                          false, false, module);
    // Create package with symbol
    std::vector<flatbuffers::Offset<IdxFormat::Symbol>> symbols = {symbol};
    auto symbolSlab = builder.CreateVector(symbols);
    // Create Sym2Ref
    std::vector<flatbuffers::Offset<IdxFormat::Ref>> refs = {ref};
    auto refsVec = builder.CreateVector(refs);
    auto sym2Ref = IdxFormat::CreateSym2Ref(builder, 1, refsVec);
    // Create package with ref
    std::vector<flatbuffers::Offset<IdxFormat::Sym2Ref>> sym2Refs = {sym2Ref};
    auto refSlab = builder.CreateVector(sym2Refs);

    // Create package with relation
    auto relationSlab = builder.CreateVector(relations);

    // Create extend slab
    auto interfaceName = builder.CreateString("TestInterface");
    auto extend = IdxFormat::CreateExtend(builder, 1, static_cast<uint8_t>(Modifier::PUBLIC), interfaceName);

    std::vector<flatbuffers::Offset<IdxFormat::Extend>> extends = {extend};
    auto extendsVec = builder.CreateVector(extends);

    auto sym2Extend = IdxFormat::CreateSym2Extend(builder, 1, extendsVec);

    std::vector<flatbuffers::Offset<IdxFormat::Sym2Extend>> sym2Extends = {sym2Extend};
    auto extendSlab = builder.CreateVector(sym2Extends);

    auto package = IdxFormat::CreateHashedPackage(builder, symbolSlab, refSlab, relationSlab, extendSlab, crossSymbolSlab);
    builder.Finish(package);

    // Get the actual cross symbol from the buffer
    auto pkg = IdxFormat::GetHashedPackage(builder.GetBufferPointer());
    auto actualCrossSymbol = pkg->cross_symbol_slab()->Get(0);

    CrossSymbol outCrossSymbol;
    EXPECT_NO_THROW(ReadCrossSymbol(outCrossSymbol, actualCrossSymbol));
}

// Test StoreRef function
TEST(IndexStorageTest, StoreRef_ValidData) {
    flatbuffers::FlatBufferBuilder builder;
    Ref ref;
    ref.kind = RefKind::REFERENCE;
    ref.container = 1;
    ref.isCjoRef = true;

    // 补全位置信息
    ref.location.begin.fileID = 1;
    ref.location.begin.line = 10;
    ref.location.begin.column = 5;
    ref.location.end.fileID = 1;
    ref.location.end.line = 10;
    ref.location.end.column = 25;
    ref.location.fileUri = "file:///test/module.cj";

    EXPECT_NO_THROW(StoreRef(builder, ref));
}


// Test StoreExtend function
TEST(IndexStorageTest, StoreExtend_ValidData) {
    flatbuffers::FlatBufferBuilder builder;
    ExtendItem extendItem;
    extendItem.id = 1;
    extendItem.modifier = Modifier::PUBLIC;
    extendItem.interfaceName = "TestInterface";

    EXPECT_NO_THROW(StoreExtend(builder, extendItem));
}

// Test StoreRelation function
TEST(IndexStorageTest, StoreRelation_ValidData) {
    flatbuffers::FlatBufferBuilder builder;
    Relation relation;
    relation.subject = 1;
    relation.predicate = RelationKind::EXTEND;
    relation.object = 2;

    EXPECT_NO_THROW(StoreRelation(builder, relation));
}

// Test StoreCrossSymbol function
TEST(IndexStorageTest, StoreCrossSymbol_ValidData) {
    flatbuffers::FlatBufferBuilder builder;
    CrossSymbol crossSymbol;
    crossSymbol.id = 1;
    crossSymbol.name = "testCrossSymbol";
    crossSymbol.crossType = CrossType::ARK_TS_WITH_INTEROP;
    crossSymbol.container = 2;
    crossSymbol.containerName = "testContainer";

    // 补全位置信息
    crossSymbol.location.begin.fileID = 1;
    crossSymbol.location.begin.line = 10;
    crossSymbol.location.begin.column = 5;
    crossSymbol.location.end.fileID = 1;
    crossSymbol.location.end.line = 10;
    crossSymbol.location.end.column = 25;
    crossSymbol.location.fileUri = "file:///test/module.cj";

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
