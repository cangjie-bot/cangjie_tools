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

// Helper function to create a properly initialized SrcIdentifier
SrcIdentifier CreateSrcIdentifier(const std::string& name, const Position& begin, const Position& end) {
    SrcIdentifier identifier;
    identifier.SetPos(begin, end);
    return identifier;
}

// Helper function to create a properly initialized Identifier
Identifier CreateIdentifier(const std::string& name, const Position& begin, const Position& end) {
    Identifier identifier;
    identifier.SetPos(begin, end);
    return identifier;
}

TEST_F(SemanticHighlightImplTest, GetFuncDecl_PrimaryConstructor) {
    auto funcDecl = std::make_shared<FuncDecl>();
    funcDecl->EnableAttr(Cangjie::AST::Attribute::PRIMARY_CONSTRUCTOR);
    funcDecl->begin = Position{1, 1, 1};
    funcDecl->end = Position{1, 1, 5};
    funcDecl->identifier = CreateSrcIdentifier("testFunction", Position{1, 1, 1}, Position{1, 1, 13});
    funcDecl->identifierForLsp = "testFunction";

    GetFuncDecl(funcDecl.get(), result, tokens, sourceManager);
    EXPECT_TRUE(result.empty());
}

TEST_F(SemanticHighlightImplTest, GetFuncDecl_InvalidIdentifier) {
    auto funcDecl = std::make_shared<FuncDecl>();
    funcDecl->EnableAttr(Cangjie::AST::Attribute::PRIMARY_CONSTRUCTOR);
    funcDecl->begin = Position{1, 1, 1};
    funcDecl->end = Position{1, 1, 5};
    funcDecl->identifier = CreateSrcIdentifier("<invalid identifier>", Position{1, 1, 1}, Position{1, 1, 20});
    funcDecl->identifierForLsp = "<invalid identifier>";

    GetFuncDecl(funcDecl.get(), result, tokens, sourceManager);
    EXPECT_TRUE(result.empty());
}

TEST_F(SemanticHighlightImplTest, GetPrimaryDecl_NormalCase) {
    auto node = std::make_shared<PrimaryCtorDecl>();
    node->identifier = CreateSrcIdentifier("PrimaryType", Position{1, 1, 1}, Position{1, 1, 12});
    node->identifierForLsp = "PrimaryType";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetPrimaryDecl(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetVarDecl_NormalCase) {
    auto node = std::make_shared<VarDecl>();
    node->identifier = CreateSrcIdentifier("testVariable", Position{1, 1, 1}, Position{1, 1, 13});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetVarDecl(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

TEST_F(SemanticHighlightImplTest, GetPropDecl_NormalCase) {
    auto node = std::make_shared<PropDecl>();
    node->identifier = CreateSrcIdentifier("testProperty", Position{1, 1, 1}, Position{1, 1, 13});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetPropDecl(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

TEST_F(SemanticHighlightImplTest, GetCallExpr_NormalCase) {
    auto node = std::make_shared<CallExpr>();
    auto symbol = Ptr<Symbol>();
    node->symbol = symbol.get();

    auto resolvedFunction = std::make_shared<FuncDecl>();
    resolvedFunction->identifier = CreateSrcIdentifier("testFunction", Position{1, 1, 1}, Position{1, 1, 13});
    node->resolvedFunction = resolvedFunction.get();

    auto baseFunc = std::make_shared<RefExpr>();
    node->baseFunc = OwnedPtr<Expr>(baseFunc.get());

    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};
    node->leftParenPos = Position{1, 1, 14};

    GetCallExpr(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::FUNCTION_H);
}

TEST_F(SemanticHighlightImplTest, GetCallExpr_MemberAccessBase) {
    auto node = std::make_shared<CallExpr>();
    auto symbol = Ptr<Symbol>();
    node->symbol = symbol.get();

    auto resolvedFunction = std::make_shared<FuncDecl>();
    resolvedFunction->identifier = CreateSrcIdentifier("testMethod", Position{1, 1, 1}, Position{1, 1, 11});
    node->resolvedFunction = resolvedFunction.get();

    auto baseFunc = std::make_shared<MemberAccess>();
    node->baseFunc = OwnedPtr<Expr>(baseFunc.get());

    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};
    node->leftParenPos = Position{1, 1, 14};

    GetCallExpr(node.get(), result, tokens, sourceManager);
    EXPECT_TRUE(result.empty());
}

TEST_F(SemanticHighlightImplTest, GetMemberAccess_ClassMember) {
    auto node = std::make_shared<MemberAccess>();
    node->field = CreateSrcIdentifier("memberField", Position{1, 1, 1}, Position{1, 1, 12});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    auto target = std::make_shared<ClassDecl>();
    target->identifier = CreateSrcIdentifier("TestClass", Position{1, 1, 1}, Position{1, 1, 10});
    node->target = target.get();

    GetMemberAccess(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetMemberAccess_PackageMember) {
    auto node = std::make_shared<MemberAccess>();
    node->field = CreateSrcIdentifier("packageName", Position{1, 1, 1}, Position{1, 1, 12});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    auto target = std::make_shared<PackageDecl>(*std::make_shared<Package>().get());
    node->target = target.get();

    GetMemberAccess(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::PACKAGE_H);
}

TEST_F(SemanticHighlightImplTest, GetFuncArg_NormalCase) {
    auto node = std::make_shared<FuncArg>();
    node->name = CreateSrcIdentifier("argName", Position{1, 1, 1}, Position{1, 1, 8});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetFuncArg(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

TEST_F(SemanticHighlightImplTest, GetFuncArg_ZeroPosition) {
    auto node = std::make_shared<FuncArg>();
    node->name = CreateSrcIdentifier("argName", Position{0, 0, 0}, Position{0, 0, 0});
    node->begin = Position{0, 0, 0};
    node->end = Position{0, 0, 0};

    GetFuncArg(node.get(), result, tokens, sourceManager);
    EXPECT_TRUE(result.empty());
}

TEST_F(SemanticHighlightImplTest, RefTargetEmpty_NullTarget) {
    auto node = std::make_shared<RefExpr>();
    // Do not set ref.target
    bool isEmpty = RefTargetEmpty(node.get());
    EXPECT_TRUE(isEmpty);
}

TEST_F(SemanticHighlightImplTest, RefTargetEmpty_NonRefExpr) {
    auto node = std::make_shared<FuncDecl>();
    bool isEmpty = RefTargetEmpty(node.get());
    EXPECT_TRUE(isEmpty);
}

TEST_F(SemanticHighlightImplTest, SpecialTarget_ClassTarget) {
    auto node = std::make_shared<RefExpr>();
    auto target = std::make_shared<ClassDecl>();
    target->identifier = CreateSrcIdentifier("TestClass", Position{1, 1, 1}, Position{1, 1, 10});
    node->ref.target = target.get();
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    bool isSpecial = SpecialTarget(node.get());
    EXPECT_TRUE(isSpecial);
}

TEST_F(SemanticHighlightImplTest, SpecialTarget_InitFunction) {
    auto node = std::make_shared<RefExpr>();
    auto target = std::make_shared<FuncDecl>();
    target->identifier = CreateSrcIdentifier("init", Position{1, 1, 1}, Position{1, 1, 5});
    node->ref.target = target.get();
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 5};

    bool isSpecial = SpecialTarget(node.get());
    EXPECT_TRUE(isSpecial);
}

TEST_F(SemanticHighlightImplTest, HandleInterpolationExpr_DollarSign) {
    delete sourceManager;
    sourceManager = new Cangjie::SourceManager();
    std::string testCode = "$variable";
    sourceManager->AddSource("test.cj", testCode);

    ark::Range range;
    range.start = Position{1, 1, 1};
    range.end = Position{1, 1, 10};

    HandleInterpolationExpr(range, sourceManager);
    EXPECT_EQ(range.start.column, 2);
    EXPECT_EQ(range.end.column, 11);
}

TEST_F(SemanticHighlightImplTest, HandleInterpolationExpr_NoSourceManager) {
    ark::Range range;
    range.start = Position{1, 1, 1};
    range.end = Position{1, 1, 10};

    HandleInterpolationExpr(range, nullptr);
    EXPECT_EQ(range.start.column, 1);
    EXPECT_EQ(range.end.column, 10);
}

TEST_F(SemanticHighlightImplTest, GetRefExpr_ClassReference) {
    auto node = std::make_shared<RefExpr>();
    auto target = std::make_shared<ClassDecl>();
    target->identifier = CreateSrcIdentifier("TestClass", Position{1, 1, 1}, Position{1, 1, 10});
    node->ref.target = target.get();
    node->ref.identifier = CreateSrcIdentifier("TestClass", Position{1, 1, 1}, Position{1, 1, 10});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetRefExpr(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetRefExpr_FunctionReference) {
    auto node = std::make_shared<RefExpr>();
    auto target = std::make_shared<FuncDecl>();
    target->identifier = CreateSrcIdentifier("testFunction", Position{1, 1, 1}, Position{1, 1, 13});
    node->ref.target = target.get();
    node->ref.identifier = CreateSrcIdentifier("testFunction", Position{1, 1, 1}, Position{1, 1, 13});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetRefExpr(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::FUNCTION_H);
}

TEST_F(SemanticHighlightImplTest, GetRefExpr_InitFunctionReference) {
    auto node = std::make_shared<RefExpr>();
    auto target = std::make_shared<FuncDecl>();
    target->identifier = CreateSrcIdentifier("init", Position{1, 1, 1}, Position{1, 1, 5});
    node->ref.target = target.get();
    node->ref.identifier = CreateSrcIdentifier("init", Position{1, 1, 1}, Position{1, 1, 5});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 5};

    GetRefExpr(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetClassDecl_NormalCase) {
    auto node = std::make_shared<ClassDecl>();
    node->identifier = CreateSrcIdentifier("TestClass", Position{1, 1, 1}, Position{1, 1, 10});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetClassDecl(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetClassDecl_InvalidIdentifier) {
    auto node = std::make_shared<ClassDecl>();
    node->identifier = CreateSrcIdentifier("<invalid identifier>", Position{1, 1, 1}, Position{1, 1, 20});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetClassDecl(node.get(), result, tokens, sourceManager);
    EXPECT_TRUE(result.empty());
}

TEST_F(SemanticHighlightImplTest, GetRefType_ClassType) {
    auto node = std::make_shared<RefType>();
    auto target = std::make_shared<ClassDecl>();
    target->identifier = CreateSrcIdentifier("TestClass", Position{1, 1, 1}, Position{1, 1, 10});
    node->ref.target = target.get();
    node->ref.identifier = CreateSrcIdentifier("TestClass", Position{1, 1, 1}, Position{1, 1, 10});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetRefType(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetRefType_InterfaceType) {
    auto node = std::make_shared<RefType>();
    auto target = std::make_shared<InterfaceDecl>();
    target->identifier = CreateSrcIdentifier("TestInterface", Position{1, 1, 1}, Position{1, 1, 14});
    node->ref.target = target.get();
    node->ref.identifier = CreateSrcIdentifier("TestInterface", Position{1, 1, 1}, Position{1, 1, 14});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetRefType(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::INTERFACE_H);
}

TEST_F(SemanticHighlightImplTest, GetFuncParam_NormalCase) {
    auto node = std::make_shared<FuncParam>();
    node->identifier = CreateSrcIdentifier("paramName", Position{1, 1, 1}, Position{1, 1, 10});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};
    node->isIdentifierCompilerAdd = false;

    GetFuncParam(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

TEST_F(SemanticHighlightImplTest, GetFuncParam_CompilerAdded) {
    auto node = std::make_shared<FuncParam>();
    node->identifier = CreateSrcIdentifier("paramName", Position{1, 1, 1}, Position{1, 1, 10});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};
    node->isIdentifierCompilerAdd = true;

    GetFuncParam(node.get(), result, tokens, sourceManager);
    EXPECT_TRUE(result.empty());
}

TEST_F(SemanticHighlightImplTest, GetInterfaceDecl_NormalCase) {
    auto node = std::make_shared<InterfaceDecl>();
    node->identifier = CreateSrcIdentifier("TestInterface", Position{1, 1, 1}, Position{1, 1, 14});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetInterfaceDecl(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::INTERFACE_H);
}

TEST_F(SemanticHighlightImplTest, GetStructDecl_NormalCase) {
    auto node = std::make_shared<StructDecl>();
    node->identifier = CreateSrcIdentifier("TestStruct", Position{1, 1, 1}, Position{1, 1, 11});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 12};

    GetStructDecl(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetEnumDecl_NormalCase) {
    auto node = std::make_shared<EnumDecl>();
    node->identifier = CreateSrcIdentifier("TestEnum", Position{1, 1, 1}, Position{1, 1, 9});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetEnumDecl(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetGenericParam_NormalCase) {
    auto node = std::make_shared<GenericParamDecl>();
    node->identifier = CreateSrcIdentifier("T", Position{1, 1, 1}, Position{1, 1, 2});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 2};

    GetGenericParam(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

TEST_F(SemanticHighlightImplTest, GetGenericParam_InvalidIdentifier) {
    auto node = std::make_shared<GenericParamDecl>();
    node->identifier = CreateSrcIdentifier("<invalid identifier>", Position{1, 1, 1}, Position{1, 1, 20});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetGenericParam(node.get(), result, tokens, sourceManager);
    EXPECT_TRUE(result.empty());
}

TEST_F(SemanticHighlightImplTest, GetQualifiedType_PackageType) {
    auto node = std::make_shared<QualifiedType>();
    auto target = std::make_shared<PackageDecl>(*std::make_shared<Package>().get());
    node->target = target.get();
    node->field = CreateSrcIdentifier("packageName", Position{1, 1, 1}, Position{1, 1, 12});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetQualifiedType(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::PACKAGE_H);
}

TEST_F(SemanticHighlightImplTest, GetQualifiedType_ClassType) {
    auto node = std::make_shared<QualifiedType>();
    auto target = std::make_shared<ClassDecl>();
    target->identifier = CreateSrcIdentifier("ClassName", Position{1, 1, 1}, Position{1, 1, 10});
    node->target = target.get();
    node->field = CreateSrcIdentifier("ClassName", Position{1, 1, 1}, Position{1, 1, 10});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetQualifiedType(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, GetTypeAliasDecl_NormalCase) {
    auto node = std::make_shared<TypeAliasDecl>();
    node->identifier = CreateSrcIdentifier("TypeAlias", Position{1, 1, 1}, Position{1, 1, 10});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetTypeAliasDecl(node.get(), result, tokens, sourceManager);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

TEST_F(SemanticHighlightImplTest, FindCharKeyWord_Keyword) {
    bool found = FindCharKeyWord("if");
    EXPECT_TRUE(found);
}

TEST_F(SemanticHighlightImplTest, FindCharKeyWord_NonKeyword) {
    bool found = FindCharKeyWord("customIdentifier");
    EXPECT_FALSE(found);
}

TEST_F(SemanticHighlightImplTest, FindCharKeyWord_KeywordIdentifier) {
    bool found = FindCharKeyWord("public");
    EXPECT_FALSE(found);
}

TEST_F(SemanticHighlightImplTest, NodeValid_ValidNode) {
    auto node = std::make_shared<FuncDecl>();
    node->identifier = CreateSrcIdentifier("validName", Position{1, 1, 1}, Position{1, 1, 9});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    bool valid = SemanticHighlightImpl::NodeValid(node.get(), 1, "validName");
    EXPECT_TRUE(valid);
}

TEST_F(SemanticHighlightImplTest, NodeValid_WrongFileId) {
    auto node = std::make_shared<FuncDecl>();
    node->identifier = CreateSrcIdentifier("validName", Position{2, 1, 1}, Position{2, 1, 9});
    node->begin = Position{2, 1, 1};
    node->end = Position{2, 1, 10};

    bool valid = SemanticHighlightImpl::NodeValid(node.get(), 1, "validName");
    EXPECT_FALSE(valid);
}

TEST_F(SemanticHighlightImplTest, NodeValid_KeywordName) {
    auto node = std::make_shared<FuncDecl>();
    node->identifier = CreateSrcIdentifier("if", Position{1, 1, 1}, Position{1, 1, 3});
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    bool valid = SemanticHighlightImpl::NodeValid(node.get(), 1, "if");
    EXPECT_FALSE(valid);
}

TEST_F(SemanticHighlightImplTest, NodeValid_ZeroPosition) {
    auto node = std::make_shared<FuncDecl>();
    node->identifier = CreateSrcIdentifier("validName", Position{0, 0, 0}, Position{0, 0, 0});
    node->begin = Position{0, 0, 0};
    node->end = Position{0, 0, 0};

    bool valid = SemanticHighlightImpl::NodeValid(node.get(), 1, "validName");
    EXPECT_FALSE(valid);
}