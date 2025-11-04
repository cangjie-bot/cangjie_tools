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

    // 创建与测试代码对应的有效 token
    Cangjie::Token varToken(
        Cangjie::TokenKind::IDENTIFIER,
        "testVariable",
        Cangjie::Position{1, 1, 1},
        Cangjie::Position{1, 1, 13}
    );
    tokens.push_back(varToken);

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
    auto decl = Ptr<Decl>(new Decl());
    decl->identifier = "TestClass";

    auto annotation = OwnedPtr<Annotation>(new Annotation());
    annotation->identifier = "@TestAnnotation";
    auto baseExpr = new Expr();
    baseExpr->begin = Position{1, 1, 1};
    baseExpr->end = Position{1, 1, 15};
    annotation->baseExpr = OwnedPtr<Expr>(baseExpr);

    decl->annotations.push_back(std::move(annotation));

    AddAnnoToken(decl, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetFuncDecl_NormalFunction) {
    auto node = Ptr<Node>(new Decl());
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetFuncDecl(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::FUNCTION_H);
}

TEST_F(SemanticHighlightImplTest, GetFuncDecl_PrimaryConstructor) {
    auto funcDecl = std::make_shared<FuncDecl>();
    funcDecl->EnableAttr(Cangjie::AST::Attribute::PRIMARY_CONSTRUCTOR);
    funcDecl->begin = Position{1, 1, 1};
    funcDecl->end = Position{1, 1, 5};

    funcDecl->identifier = "testFunction";
    funcDecl->identifierForLsp = "testFunction";

    GetFuncDecl(funcDecl.get(), result, tokens, sourceManager);
    EXPECT_TRUE(result.empty());
}

TEST_F(SemanticHighlightImplTest, GetFuncDecl_InvalidIdentifier) {
    auto funcDecl = std::make_shared<FuncDecl>();
    funcDecl->EnableAttr(Cangjie::AST::Attribute::PRIMARY_CONSTRUCTOR);
    funcDecl->begin = Position{1, 1, 1};
    funcDecl->end = Position{1, 1, 5};

    funcDecl->identifier = "<invalid identifier>";
    funcDecl->identifierForLsp = "<invalid identifier>";

    GetFuncDecl(funcDecl.get(), result, tokens, sourceManager);

    EXPECT_TRUE(result.empty());
}

TEST_F(SemanticHighlightImplTest, GetPrimaryDecl_NormalCase) {
    auto node = std::make_shared<PrimaryCtorDecl>();

    node->identifier = "PrimaryType";
    node->identifierForLsp = "PrimaryType";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetPrimaryDecl(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetVarDecl_NormalCase) {
    auto node = std::make_shared<VarDecl>();
    node->identifier = "testVariable";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetVarDecl(node.get(), result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

TEST_F(SemanticHighlightImplTest, GetPropDecl_NormalCase) {
    auto node = Ptr<PropDecl>();
    node->identifier = "testProperty";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetPropDecl(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

TEST_F(SemanticHighlightImplTest, GetCallExpr_NormalCase) {
    auto node = Ptr<CallExpr>();
    auto symbol = Ptr<Symbol>();
    node->symbol = symbol;

    auto resolvedFunction = Ptr<FuncDecl>();
    resolvedFunction->identifier = "testFunction";
    node->resolvedFunction = resolvedFunction;

    auto baseFunc = Ptr<RefExpr>();
    node->baseFunc = OwnedPtr<Expr>(baseFunc);

    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};
    node->leftParenPos = Position{1, 1, 14};

    GetCallExpr(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::FUNCTION_H);
}

TEST_F(SemanticHighlightImplTest, GetCallExpr_MemberAccessBase) {
    auto node = Ptr<CallExpr>();

    auto symbol = Ptr<Symbol>();
    node->symbol = symbol;

    auto resolvedFunction = Ptr<FuncDecl>();
    resolvedFunction->identifier = "testMethod";
    node->resolvedFunction = resolvedFunction;

    auto baseFunc = new Expr();
    node->baseFunc = OwnedPtr<Expr>(baseFunc);

    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};
    node->leftParenPos = Position{1, 1, 14};

    GetCallExpr(node, result, tokens, sourceManager);

    EXPECT_TRUE(result.empty());
}

TEST_F(SemanticHighlightImplTest, GetMemberAccess_ClassMember) {
    auto node = Ptr<MemberAccess>();
    node->field = "memberField";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    auto target = Ptr<ClassDecl>();
    target->identifier = "TestClass";
    node->target = target;

    GetMemberAccess(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetMemberAccess_PackageMember) {
    auto node = Ptr<MemberAccess>();
    node->field = "packageName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    auto target = Ptr<PackageDecl>();
    node->target = target;

    GetMemberAccess(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::PACKAGE_H);
}

TEST_F(SemanticHighlightImplTest, GetFuncArg_NormalCase) {
    auto node = Ptr<FuncArg>();
    node->name = "argName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetFuncArg(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

TEST_F(SemanticHighlightImplTest, GetFuncArg_ZeroPosition) {
    auto node = Ptr<FuncArg>();
    node->name = "argName";
    node->begin = Position{0, 0, 0};
    node->end = Position{0, 0, 0};

    GetFuncArg(node, result, tokens, sourceManager);

    EXPECT_TRUE(result.empty());
}

// Test RefTargetEmpty function
TEST_F(SemanticHighlightImplTest, RefTargetEmpty_NullTarget) {
    auto node = Ptr<RefExpr>();
    // Do not set ref.target

    bool isEmpty = RefTargetEmpty(node);

    EXPECT_TRUE(isEmpty);
}

// Test RefTargetEmpty function - Non-RefExpr node
TEST_F(SemanticHighlightImplTest, RefTargetEmpty_NonRefExpr) {
    auto node = Ptr<Decl>();
    // Pass in non-RefExpr node

    bool isEmpty = RefTargetEmpty(node);

    EXPECT_TRUE(isEmpty);
}

// Test SpecialTarget function
TEST_F(SemanticHighlightImplTest, SpecialTarget_ClassTarget) {
    auto node = Ptr<RefExpr>();
    auto target = Ptr<ClassDecl>();
    node->ref.target = target;

    bool isSpecial = SpecialTarget(node);

    EXPECT_TRUE(isSpecial);
}

// Test SpecialTarget function - Init function
TEST_F(SemanticHighlightImplTest, SpecialTarget_InitFunction) {
    auto node = Ptr<RefExpr>();
    auto target = Ptr<FuncDecl>();
    target->identifier = "init";
    node->ref.target = target;

    bool isSpecial = SpecialTarget(node);

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
    auto node = Ptr<RefExpr>();
    auto target = Ptr<ClassDecl>();
    target->identifier = "TestClass";
    node->ref.target = target;
    // Set other required fields
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetRefExpr(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test GetRefExpr function - Function reference
TEST_F(SemanticHighlightImplTest, GetRefExpr_FunctionReference) {
    auto node = Ptr<RefExpr>();
    auto target = Ptr<FuncDecl>();
    target->identifier = "testFunction";
    node->ref.target = target;
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetRefExpr(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::FUNCTION_H);
}

// Test GetRefExpr function - Init function reference
TEST_F(SemanticHighlightImplTest, GetRefExpr_InitFunctionReference) {
    auto node = Ptr<RefExpr>();
    auto target = Ptr<FuncDecl>();
    target->identifier = "init";
    node->ref.target = target;
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 5};

    GetRefExpr(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test GetClassDecl function
TEST_F(SemanticHighlightImplTest, GetClassDecl_NormalCase) {
    auto node = Ptr<ClassDecl>();
    node->identifier = "TestClass";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetClassDecl(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test GetClassDecl function - Invalid identifier
TEST_F(SemanticHighlightImplTest, GetClassDecl_InvalidIdentifier) {
    auto node = Ptr<ClassDecl>();
    node->identifier = "<invalid identifier>";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetClassDecl(node, result, tokens, sourceManager);

    // Invalid identifiers should not produce highlight tokens
    EXPECT_TRUE(result.empty());
}

// Test GetRefType function
TEST_F(SemanticHighlightImplTest, GetRefType_ClassType) {
    auto node = Ptr<RefType>();
    auto target = Ptr<ClassDecl>();
    node->ref.target = target;
    node->ref.identifier = "TestClass";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetRefType(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test GetRefType function - Interface type
TEST_F(SemanticHighlightImplTest, GetRefType_InterfaceType) {
    auto node = Ptr<RefType>();
    auto target = Ptr<InterfaceDecl>();
    node->ref.target = target;
    node->ref.identifier = "TestInterface";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetRefType(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::INTERFACE_H);
}

// Test GetFuncParam function
TEST_F(SemanticHighlightImplTest, GetFuncParam_NormalCase) {
    auto node = Ptr<FuncParam>();
    node->identifier = "paramName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};
    // Do not set isIdentifierCompilerAdd or set it to false

    GetFuncParam(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

// Test GetFuncParam function - Compiler-added identifier
TEST_F(SemanticHighlightImplTest, GetFuncParam_CompilerAdded) {
    auto node = Ptr<FuncParam>();
    node->identifier = "paramName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};
    node->isIdentifierCompilerAdd = true;

    GetFuncParam(node, result, tokens, sourceManager);

    // Compiler-added identifiers should not produce highlight tokens
    EXPECT_TRUE(result.empty());
}

// Test GetInterfaceDecl function
TEST_F(SemanticHighlightImplTest, GetInterfaceDecl_NormalCase) {
    auto node = Ptr<InterfaceDecl>();
    node->identifier = "TestInterface";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetInterfaceDecl(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::INTERFACE_H);
}

// Test GetStructDecl function
TEST_F(SemanticHighlightImplTest, GetStructDecl_NormalCase) {
    auto node = Ptr<StructDecl>();
    node->identifier = "TestStruct";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 12};

    GetStructDecl(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test GetEnumDecl function
TEST_F(SemanticHighlightImplTest, GetEnumDecl_NormalCase) {
    auto node = Ptr<EnumDecl>();
    node->identifier = "TestEnum";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetEnumDecl(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test GetGenericParam function
TEST_F(SemanticHighlightImplTest, GetGenericParam_NormalCase) {
    auto node = Ptr<GenericParamDecl>();
    node->identifier = "T";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 2};

    GetGenericParam(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

// Test GetGenericParam function - Invalid identifier
TEST_F(SemanticHighlightImplTest, GetGenericParam_InvalidIdentifier) {
    auto node = Ptr<GenericParamDecl>();
    node->identifier = "<invalid identifier>";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetGenericParam(node, result, tokens, sourceManager);

    // Invalid identifiers should not produce highlight tokens
    EXPECT_TRUE(result.empty());
}

// Test GetQualifiedType function
TEST_F(SemanticHighlightImplTest, GetQualifiedType_PackageType) {
    auto node = Ptr<QualifiedType>();
    auto target = Ptr<PackageDecl>();
    node->target = target;
    node->field = "packageName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetQualifiedType(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::PACKAGE_H);
}

// Test GetQualifiedType function - Class type
TEST_F(SemanticHighlightImplTest, GetQualifiedType_ClassType) {
    auto node = Ptr<QualifiedType>();
    auto target = Ptr<ClassDecl>();
    node->target = target;
    node->field = "ClassName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetQualifiedType(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// Test GetTypeAliasDecl function
TEST_F(SemanticHighlightImplTest, GetTypeAliasDecl_NormalCase) {
    auto node = Ptr<TypeAliasDecl>();
    node->identifier = "TypeAlias";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetTypeAliasDecl(node, result, tokens, sourceManager);

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
    auto node = Ptr<Decl>();
    // Set correct file ID and position
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    bool valid = SemanticHighlightImpl::NodeValid(node, 1, "validName");

    EXPECT_TRUE(valid);
}

// Test SemanticHighlightImpl::NodeValid function - Wrong file ID
TEST_F(SemanticHighlightImplTest, NodeValid_WrongFileId) {
    auto node = Ptr<Decl>();
    // Set incorrect file ID
    node->begin = Position{2, 1, 1}; // File ID is 2
    node->end = Position{2, 1, 10};

    bool valid = SemanticHighlightImpl::NodeValid(node, 1, "validName"); // Expected file ID is 1

    EXPECT_FALSE(valid);
}

// Test SemanticHighlightImpl::NodeValid function - Keyword name
TEST_F(SemanticHighlightImplTest, NodeValid_KeywordName) {
    auto node = Ptr<Decl>();
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    bool valid = SemanticHighlightImpl::NodeValid(node, 1, "if"); // if is a keyword

    EXPECT_FALSE(valid);
}

// Test SemanticHighlightImpl::NodeValid function - Zero position
TEST_F(SemanticHighlightImplTest, NodeValid_ZeroPosition) {
    auto node = Ptr<Decl>();
    // Do not set position or set to zero position
    node->begin = Position{0, 0, 0};
    node->end = Position{0, 0, 0};

    bool valid = SemanticHighlightImpl::NodeValid(node, 1, "validName");

    EXPECT_FALSE(valid);
}
