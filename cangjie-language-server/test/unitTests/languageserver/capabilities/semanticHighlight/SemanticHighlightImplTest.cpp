// SemanticHighlightImplTest.cpp
// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "SemanticHighlightImpl.cpp"
#include <vector>
#include <string>
#include "cangjie/Basic/SourceManager.h"

using namespace ark;
using namespace Cangjie;
using namespace Cangjie::AST;

// Helper function to create test tokens
std::vector<Cangjie::Token> CreateTestTokens() {
    std::vector<Cangjie::Token> tokens;
    // 添加一些测试用的Token
    tokens.emplace_back(TokenKind::IDENTIFIER, "test");
    tokens.emplace_back(TokenKind::STRING_LITERAL, "\"hello\"");
    return tokens;
}

// Helper function to create test source manager
Cangjie::SourceManager* CreateTestSourceManager() {
    auto* sourceManager = new Cangjie::SourceManager();
    std::string testCode = "func testFunction() {\n"
                           "    var testVariable = 10\n"
                           "    $testVariable\n"
                           "}\n";
    sourceManager->AddSource("test.cj", testCode);
    return sourceManager;
}

class SemanticHighlightImplTest : public ::testing::Test {
protected:
    void SetUp() override {
        sourceManager = CreateTestSourceManager();
        tokens = CreateTestTokens();
        result.clear();
    }

    void TearDown() override {
        delete sourceManager;
    }

    std::vector<Cangjie::Token> tokens;
    Cangjie::SourceManager* sourceManager;
    std::vector<SemanticHighlightToken> result;
};

TEST_F(SemanticHighlightImplTest, AddAnnoToken_NormalCase) {
    auto decl = MakeOwned<Decl>();
    decl->identifier = "TestClass";

    auto annotation = MakeOwned<Annotation>();
    annotation->identifier = "TestAnnotation";
    auto baseExpr = MakeOwned<Expr>();
    baseExpr->begin = Position{1, 1, 1};
    baseExpr->end = Position{1, 1, 15};
    annotation->baseExpr = std::move(baseExpr);

    decl->annotations.push_back(std::move(annotation));

    AddAnnoToken(decl.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetFuncDecl_NormalFunction) {
    auto node = MakeOwned<FuncDecl>();
    node->identifier = "testFunction";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetFuncDecl(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::FUNCTION_H);
}

TEST_F(SemanticHighlightImplTest, GetFuncDecl_PrimaryConstructor) {
    auto node = MakeOwned<FuncDecl>();
    node->EnableAttr(Cangjie::AST::Attribute::PRIMARY_CONSTRUCTOR);
    node->identifier = "PrimaryConstructor";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 5};

    GetFuncDecl(node.get(), result, tokens, sourceManager);

    EXPECT_TRUE(result.empty());
}

TEST_F(SemanticHighlightImplTest, GetFuncDecl_InvalidIdentifier) {
    auto node = MakeOwned<FuncDecl>();
    node->identifier = "<invalid identifier>";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetFuncDecl(node.get(), result, tokens, sourceManager);

    EXPECT_TRUE(result.empty());
}

TEST_F(SemanticHighlightImplTest, GetPrimaryDecl_NormalCase) {
    auto node = MakeOwned<PrimaryCtorDecl>();
    node->identifier = "PrimaryType";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetPrimaryDecl(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetVarDecl_NormalCase) {
    auto node = MakeOwned<VarDecl>();
    node->identifier = "testVariable";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetVarDecl(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

TEST_F(SemanticHighlightImplTest, GetPropDecl_NormalCase) {
    auto node = MakeOwned<PropDecl>();
    node->identifier = "testProperty";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetPropDecl(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

TEST_F(SemanticHighlightImplTest, GetCallExpr_NormalCase) {
    auto node = MakeOwned<CallExpr>();
    auto symbol = OwnedPtr<Symbol>();
    node->symbol = symbol.get();

    auto resolvedFunction = MakeOwned<FuncDecl>();
    resolvedFunction->identifier = "testFunction";
    node->resolvedFunction = resolvedFunction.get();

    auto baseFunc = MakeOwned<RefExpr>();
    node->baseFunc = std::move(baseFunc);

    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};
    node->leftParenPos = Position{1, 1, 14};

    GetCallExpr(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::FUNCTION_H);
}

TEST_F(SemanticHighlightImplTest, GetCallExpr_MemberAccessBase) {
    auto node = MakeOwned<CallExpr>();

    auto symbol = MakeOwned<Symbol>();
    node->symbol = symbol.get();

    auto resolvedFunction = MakeOwned<FuncDecl>();
    resolvedFunction->identifier = "testMethod";
    node->resolvedFunction = resolvedFunction.get();

    auto baseFunc = MakeOwned<MemberAccess>();
    node->baseFunc = std::move(baseFunc);

    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};
    node->leftParenPos = Position{1, 1, 14};

    GetCallExpr(node.get(), result, tokens, sourceManager);

    EXPECT_TRUE(result.empty());
}

TEST_F(SemanticHighlightImplTest, GetMemberAccess_ClassMember) {
    auto node = MakeOwned<MemberAccess>();
    node->field = "memberField";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    auto target = MakeOwned<ClassDecl>();
    target->identifier = "TestClass";
    node->target = std::move(target);

    GetMemberAccess(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetMemberAccess_PackageMember) {
    auto node = MakeOwned<MemberAccess>();
    node->field = "packageName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    auto target = OwnedPtr<PackageDecl>();
    node->target = std::move(target);

    GetMemberAccess(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::PACKAGE_H);
}

TEST_F(SemanticHighlightImplTest, GetFuncArg_NormalCase) {
    auto node = MakeOwned<FuncArg>();
    node->name = "argName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetFuncArg(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

TEST_F(SemanticHighlightImplTest, GetFuncArg_ZeroPosition) {
    auto node = MakeOwned<FuncArg>();
    node->name = "argName";
    node->begin = Position{0, 0, 0};
    node->end = Position{0, 0, 0};

    GetFuncArg(node.get(), result, tokens, sourceManager);

    EXPECT_TRUE(result.empty());
}

// Test RefTargetEmpty function
TEST_F(SemanticHighlightImplTest, RefTargetEmpty_NullTarget) {
    auto node = MakeOwned<RefExpr>();
    // Do not set ref.target

    bool isEmpty = RefTargetEmpty(node.get());

    EXPECT_TRUE(isEmpty);
}

// Test RefTargetEmpty function - Non-RefExpr node
TEST_F(SemanticHighlightImplTest, RefTargetEmpty_NonRefExpr) {
    auto node = MakeOwned<Decl>();
    // Pass in non-RefExpr node

    bool isEmpty = RefTargetEmpty(node.get());

    EXPECT_TRUE(isEmpty);
}

// Test SpecialTarget function
TEST_F(SemanticHighlightImplTest, SpecialTarget_ClassTarget) {
    auto node = MakeOwned<RefExpr>();
    auto target = MakeOwned<ClassDecl>();
    node->ref.target = target.get();

    bool isSpecial = SpecialTarget(node.get());

    EXPECT_TRUE(isSpecial);
}

// Test SpecialTarget function - Init function
TEST_F(SemanticHighlightImplTest, SpecialTarget_InitFunction) {
    auto node = MakeOwned<RefExpr>();
    auto target = MakeOwned<FuncDecl>();
    target->identifier = "init";
    node->ref.target = target.get();

    bool isSpecial = SpecialTarget(node.get());

    EXPECT_TRUE(isSpecial);
}

// Test HandleInterpolationExpr function
TEST_F(SemanticHighlightImplTest, HandleInterpolationExpr_DollarSign) {
    // Recreate a source manager containing $
    delete sourceManager;
    sourceManager = new Cangjie::SourceManager();
    std::string testCode = "$variable";
    sourceManager->AddSource("test.cj", testCode);

    ark::Range range;
    range.start = Position{1, 1, 1};
    range.end = Position{1, 1, 10};

    HandleInterpolationExpr(range, sourceManager);

    // Verify that the range is correctly adjusted (should offset by 1 character)
    EXPECT_EQ(range.start.column, 2);
    EXPECT_EQ(range.end.column, 11);
}

// Test HandleInterpolationExpr function - No source manager
TEST_F(SemanticHighlightImplTest, HandleInterpolationExpr_NoSourceManager) {
    ark::Range range;
    range.start = Position{1, 1, 1};
    range.end = Position{1, 1, 10};

    HandleInterpolationExpr(range, nullptr);

    // Should remain unchanged
    EXPECT_EQ(range.start.column, 1);
    EXPECT_EQ(range.end.column, 10);
}

// Test GetRefExpr function
TEST_F(SemanticHighlightImplTest, GetRefExpr_ClassReference) {
    auto node = MakeOwned<RefExpr>();
    auto target = MakeOwned<ClassDecl>();
    target->identifier = "TestClass";
    node->ref.target = target.get();
    // Set other required fields
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetRefExpr(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test GetRefExpr function - Function reference
TEST_F(SemanticHighlightImplTest, GetRefExpr_FunctionReference) {
    auto node = MakeOwned<RefExpr>();
    auto target = MakeOwned<FuncDecl>();
    target->identifier = "testFunction";
    node->ref.target = target.get();
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetRefExpr(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::FUNCTION_H);
}

// Test GetRefExpr function - Init function reference
TEST_F(SemanticHighlightImplTest, GetRefExpr_InitFunctionReference) {
    auto node = MakeOwned<RefExpr>();
    auto target = MakeOwned<FuncDecl>();
    target->identifier = "init";
    node->ref.target = target.get();
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 5};

    GetRefExpr(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test GetClassDecl function
TEST_F(SemanticHighlightImplTest, GetClassDecl_NormalCase) {
    auto node = MakeOwned<ClassDecl>();
    node->identifier = "TestClass";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetClassDecl(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test GetClassDecl function - Invalid identifier
TEST_F(SemanticHighlightImplTest, GetClassDecl_InvalidIdentifier) {
    auto node = MakeOwned<ClassDecl>();
    node->identifier = "<invalid identifier>";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetClassDecl(node.get(), result, tokens, sourceManager);

    // Invalid identifiers should not produce highlight tokens
    EXPECT_TRUE(result.empty());
}

// Test GetRefType function
TEST_F(SemanticHighlightImplTest, GetRefType_ClassType) {
    auto node = MakeOwned<RefType>();
    auto target = MakeOwned<ClassDecl>();
    node->ref.target = target.get();
    node->ref.identifier = "TestClass";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetRefType(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test GetRefType function - Interface type
TEST_F(SemanticHighlightImplTest, GetRefType_InterfaceType) {
    auto node = MakeOwned<RefType>();
    auto target = MakeOwned<InterfaceDecl>();
    node->ref.target = target.get();
    node->ref.identifier = "TestInterface";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetRefType(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::INTERFACE_H);
}

// Test GetFuncParam function
TEST_F(SemanticHighlightImplTest, GetFuncParam_NormalCase) {
    auto node = MakeOwned<FuncParam>();
    node->identifier = "paramName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};
    // Do not set isIdentifierCompilerAdd or set it to false

    GetFuncParam(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

// Test GetFuncParam function - Compiler-added identifier
TEST_F(SemanticHighlightImplTest, GetFuncParam_CompilerAdded) {
    auto node = MakeOwned<FuncParam>();
    node->identifier = "paramName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};
    node->isIdentifierCompilerAdd = true;

    GetFuncParam(node.get(), result, tokens, sourceManager);

    // Compiler-added identifiers should not produce highlight tokens
    EXPECT_TRUE(result.empty());
}

// Test GetInterfaceDecl function
TEST_F(SemanticHighlightImplTest, GetInterfaceDecl_NormalCase) {
    auto node = MakeOwned<InterfaceDecl>();
    node->identifier = "TestInterface";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetInterfaceDecl(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::INTERFACE_H);
}

// Test GetStructDecl function
TEST_F(SemanticHighlightImplTest, GetStructDecl_NormalCase) {
    auto node = MakeOwned<StructDecl>();
    node->identifier = "TestStruct";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 12};

    GetStructDecl(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test GetEnumDecl function
TEST_F(SemanticHighlightImplTest, GetEnumDecl_NormalCase) {
    auto node = MakeOwned<EnumDecl>();
    node->identifier = "TestEnum";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetEnumDecl(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test GetGenericParam function
TEST_F(SemanticHighlightImplTest, GetGenericParam_NormalCase) {
    auto node = MakeOwned<GenericParamDecl>();
    node->identifier = "T";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 2};

    GetGenericParam(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

// Test GetGenericParam function - Invalid identifier
TEST_F(SemanticHighlightImplTest, GetGenericParam_InvalidIdentifier) {
    auto node = MakeOwned<GenericParamDecl>();
    node->identifier = "<invalid identifier>";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetGenericParam(node.get(), result, tokens, sourceManager);

    // Invalid identifiers should not produce highlight tokens
    EXPECT_TRUE(result.empty());
}

// Test GetQualifiedType function
TEST_F(SemanticHighlightImplTest, GetQualifiedType_PackageType) {
    auto node = MakeOwned<QualifiedType>();
    auto target = OwnedPtr<PackageDecl>();
    node->target = target.get();
    node->field = "packageName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetQualifiedType(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::PACKAGE_H);
}

// Test GetQualifiedType function - Class type
TEST_F(SemanticHighlightImplTest, GetQualifiedType_ClassType) {
    auto node = MakeOwned<QualifiedType>();
    auto target = MakeOwned<ClassDecl>();
    node->target = target.get();
    node->field = "ClassName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetQualifiedType(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test GetTypeAliasDecl function
TEST_F(SemanticHighlightImplTest, GetTypeAliasDecl_NormalCase) {
    auto node = MakeOwned<TypeAliasDecl>();
    node->identifier = "TypeAlias";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetTypeAliasDecl(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test FindCharKeyWord function
TEST_F(SemanticHighlightImplTest, FindCharKeyWord_Keyword) {
    bool found = FindCharKeyWord("if");

    EXPECT_TRUE(found);
}

// Test FindCharKeyWord function - Non-keyword
TEST_F(SemanticHighlightImplTest, FindCharKeyWord_NonKeyword) {
    bool found = FindCharKeyWord("customIdentifier");

    EXPECT_FALSE(found);
}

// Test FindCharKeyWord function - Keyword identifier
TEST_F(SemanticHighlightImplTest, FindCharKeyWord_KeywordIdentifier) {
    bool found = FindCharKeyWord("public");

    EXPECT_FALSE(found); // public is in KEYWORD_IDENTIFIER, should return false
}

// Test SemanticHighlightImpl::NodeValid function
TEST_F(SemanticHighlightImplTest, NodeValid_ValidNode) {
    auto node = MakeOwned<Decl>();
    // Set correct file ID and position
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    bool valid = SemanticHighlightImpl::NodeValid(node.get(), 1, "validName");

    EXPECT_TRUE(valid);
}

// Test SemanticHighlightImpl::NodeValid function - Wrong file ID
TEST_F(SemanticHighlightImplTest, NodeValid_WrongFileId) {
    auto node = MakeOwned<Decl>();
    // Set incorrect file ID
    node->begin = Position{2, 1, 1}; // File ID is 2
    node->end = Position{2, 1, 10};

    bool valid = SemanticHighlightImpl::NodeValid(node.get(), 1, "validName"); // Expected file ID is 1

    EXPECT_FALSE(valid);
}

// Test SemanticHighlightImpl::NodeValid function - Keyword name
TEST_F(SemanticHighlightImplTest, NodeValid_KeywordName) {
    auto node = MakeOwned<Decl>();
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    bool valid = SemanticHighlightImpl::NodeValid(node.get(), 1, "if"); // if is a keyword

    EXPECT_FALSE(valid);
}

// Test SemanticHighlightImpl::NodeValid function - Zero position
TEST_F(SemanticHighlightImplTest, NodeValid_ZeroPosition) {
    auto node = MakeOwned<Decl>();
    // Do not set position or set to zero position
    node->begin = Position{0, 0, 0};
    node->end = Position{0, 0, 0};

    bool valid = SemanticHighlightImpl::NodeValid(node.get(), 1, "validName");

    EXPECT_FALSE(valid);
}
