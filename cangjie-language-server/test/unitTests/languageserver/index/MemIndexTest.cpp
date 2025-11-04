// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "MemIndex.h"
#include <string>
#include <unordered_set>
#include <vector>

using namespace ark::lsp;

// Test GetPackageRelation function
TEST(MemIndexTest, GetPackageRelation_Parent) {
    std::string src = "com.example";
    std::string target = "com.example.subpackage";

    PackageRelation result = GetPackageRelation(src, target);
    EXPECT_EQ(result, PackageRelation::PARENT);
}

TEST(MemIndexTest, GetPackageRelation_Child) {
    std::string src = "com.example.subpackage";
    std::string target = "com.example";

    PackageRelation result = GetPackageRelation(src, target);
    EXPECT_EQ(result, PackageRelation::CHILD);
}

TEST(MemIndexTest, GetPackageRelation_SameModule) {
    std::string src = "com.example.package1";
    std::string target = "com.example.package2";

    PackageRelation result = GetPackageRelation(src, target);
    EXPECT_EQ(result, PackageRelation::SAME_MODULE);
}

TEST(MemIndexTest, GetPackageRelation_None) {
    std::string src = "com.example.package1";
    std::string target = "org.other.package";

    PackageRelation result = GetPackageRelation(src, target);
    EXPECT_EQ(result, PackageRelation::NONE);
}

TEST(MemIndexTest, GetPackageRelation_Identical) {
    std::string src = "com.example.package";
    std::string target = "com.example.package";

    PackageRelation result = GetPackageRelation(src, target);
    EXPECT_EQ(result, PackageRelation::NONE);
}

// Test MemIndex class
TEST(MemIndexTest, MemIndex_Constructor) {
    MemIndex index;
    // Ensure object can be constructed
    EXPECT_TRUE(true);
}

TEST(MemIndexTest, MemIndex_FuzzyFind_Empty) {
    MemIndex index;
    FuzzyFindRequest req;
    req.query = "test";

    size_t callCount = 0;
    auto callback = [&callCount](const Symbol &sym) {
        callCount++;
    };

    index.FuzzyFind(req, callback);
    EXPECT_EQ(callCount, 0u);
}

TEST(MemIndexTest, MemIndex_Lookup_Empty) {
    MemIndex index;
    LookupRequest req;
    req.ids = {1, 2, 3};

    size_t callCount = 0;
    auto callback = [&callCount](const Symbol &sym) {
        callCount++;
    };

    index.Lookup(req, callback);
    EXPECT_EQ(callCount, 0u);
}

TEST(MemIndexTest, MemIndex_FindPkgSyms_Empty) {
    MemIndex index;
    PkgSymsRequest req;
    req.fullPkgName = "com.example.test";

    size_t callCount = 0;
    auto callback = [&callCount](const Symbol &sym) {
        callCount++;
    };

    index.FindPkgSyms(req, callback);
    EXPECT_EQ(callCount, 0u);
}

TEST(MemIndexTest, MemIndex_Refs_Empty) {
    MemIndex index;
    RefsRequest req;
    req.ids = {1, 2, 3};
    req.filter = RefKind::ALL;

    size_t callCount = 0;
    auto callback = [&callCount](const Ref &ref) {
        callCount++;
    };

    index.Refs(req, callback);
    EXPECT_EQ(callCount, 0u);
}

TEST(MemIndexTest, MemIndex_FileRefs_Empty) {
    MemIndex index;
    FileRefsRequest req;
    req.fullPkgName = "com.example.test";
    req.fileID = 1;
    req.filter = RefKind::ALL;

    size_t callCount = 0;
    auto callback = [&callCount](const Ref &ref, const SymbolID symId) {
        callCount++;
    };

    index.FileRefs(req, callback);
    EXPECT_EQ(callCount, 0u);
}

TEST(MemIndexTest, MemIndex_RefsFindReference_Empty) {
    MemIndex index;
    RefsRequest req;
    req.ids = {1, 2, 3};
    req.filter = RefKind::ALL;
    Ref definition;

    size_t callCount = 0;
    auto callback = [&callCount](const Ref &ref) {
        callCount++;
    };

    index.RefsFindReference(req, definition, callback);
    EXPECT_EQ(callCount, 0u);
}

TEST(MemIndexTest, MemIndex_Callees_Empty) {
    MemIndex index;
    std::string pkgName = "com.example.test";
    SymbolID declId = 1;

    size_t callCount = 0;
    auto callback = [&callCount](const SymbolID &id, const Ref &ref) {
        callCount++;
    };

    index.Callees(pkgName, declId, callback);
    EXPECT_EQ(callCount, 0u);
}

TEST(MemIndexTest, MemIndex_Relations_Empty) {
    MemIndex index;
    RelationsRequest req;
    req.id = 1;
    req.predicate = RelationKind::BASE_OF;

    size_t callCount = 0;
    auto callback = [&callCount](const Relation &rel) {
        callCount++;
    };

    index.Relations(req, callback);
    EXPECT_EQ(callCount, 0u);
}

TEST(MemIndexTest, MemIndex_GetAimSymbol_NonExistentPackage) {
    MemIndex index;
    // Create a simple Decl object for testing
    Cangjie::AST::FuncDecl decl;
    decl.fullPackageName = "com.example.nonexistent";

    Symbol result = index.GetAimSymbol(decl);
    // Should return an empty Symbol
    EXPECT_EQ(result.id, 0u);
}

TEST(MemIndexTest, MemIndex_FindImportSymsOnCompletion_TestMode) {
    MemIndex index;
    std::pair<std::unordered_set<SymbolID>, std::unordered_set<SymbolID>> filterSyms;
    std::string curPkgName = "com.example.current";
    std::string curModule = "example";
    std::string prefix = "test";

    size_t callCount = 0;
    auto callback = [&callCount](const std::string &pkgName, const Symbol &sym, const CompletionItem &item) {
        callCount++;
    };

    // In test mode, should return directly without calling callback function
    index.FindImportSymsOnCompletion(filterSyms, curPkgName, curModule, prefix, callback);
    EXPECT_EQ(callCount, 0u);
}

TEST(MemIndexTest, MemIndex_FindExtendSymsOnCompletion_TestMode) {
    MemIndex index;
    SymbolID dotCompleteSym = 1;
    std::unordered_set<SymbolID> visibleMembers = {2, 3};
    std::string curPkgName = "com.example.current";
    std::string curModule = "example";

    size_t callCount = 0;
    auto callback = [&callCount](const std::string &pkgName, const std::string &interfaceName,
                                const Symbol &sym, const CompletionItem &item) {
        callCount++;
    };

    // In test mode, should return directly without calling callback function
    index.FindExtendSymsOnCompletion(dotCompleteSym, visibleMembers, curPkgName, curModule, callback);
    EXPECT_EQ(callCount, 0u);
}

TEST(MemIndexTest, MemIndex_FindImportSymsOnQuickFix_Empty) {
    MemIndex index;
    std::string curPkgName = "com.example.current";
    std::string curModule = "example";
    std::unordered_set<SymbolID> importDeclSyms = {1, 2, 3};
    std::string identifier = "testFunction";

    size_t callCount = 0;
    auto callback = [&callCount](const std::string &pkgName, const Symbol &sym) {
        callCount++;
    };

    index.FindImportSymsOnQuickFix(curPkgName, curModule, importDeclSyms, identifier, callback);
    EXPECT_EQ(callCount, 0u);
}

TEST(MemIndexTest, MemIndex_FindComment_EmptySymbol) {  Symbol sym;
    std::vector<std::string> comments;

    // Create MemIndex instance to call non-static method
    MemIndex index;
    index.FindComment(sym, comments);
    EXPECT_TRUE(comments.empty());
}


TEST(MemIndexTest, MemIndex_FindCrossSymbolByName_Empty) {
    MemIndex index;
    std::string packageName = "com.example.test";
    std::string symName = "TestSymbol";
    bool isCombined = false;

    size_t callCount = 0;
    auto callback = [&callCount](const CrossSymbol &crs) {
        callCount++;
    };

    index.FindCrossSymbolByName(packageName, symName, isCombined, callback);
    EXPECT_EQ(callCount, 0u);
}

TEST(MemIndexTest, MemIndex_GetExportSID_Empty) {
    MemIndex index;
    IDArray array;

    size_t callCount = 0;
    auto callback = [&callCount](const CrossSymbol &crs) {
        callCount++;
    };

    index.GetExportSID(array, callback);
    EXPECT_EQ(callCount, 0u);
}
