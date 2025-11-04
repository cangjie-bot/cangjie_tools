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
    // 创建一些基本的测试 tokens
    return tokens;
}

// Helper function to create test source manager
Cangjie::SourceManager* CreateTestSourceManager() {
    auto* sourceManager = new Cangjie::SourceManager();
    // 添加一个测试源文件
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
        // 初始化测试环境
        sourceManager = CreateTestSourceManager();
        // 创建一些基本的测试 tokens
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

// 测试 AddAnnoToken 函数
TEST_F(SemanticHighlightImplTest, AddAnnoToken_NormalCase) {
    // 创建带有注解的 Decl 节点
    auto decl = Ptr<Decl>(new Decl());
    decl->identifier = "TestClass";

    // 创建注解
    auto annotation = OwnedPtr<Annotation>(new Annotation());
    annotation->identifier = "@TestAnnotation";
    auto baseExpr = new Expr();
    baseExpr->begin = Position{1, 1, 1};
    baseExpr->end = Position{1, 1, 15};
    annotation->baseExpr = OwnedPtr<Expr>(baseExpr);

    decl->annotations.push_back(std::move(annotation));

    AddAnnoToken(decl, result, tokens, sourceManager);

    // 验证结果
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// 测试 GetFuncDecl 函数
TEST_F(SemanticHighlightImplTest, GetFuncDecl_NormalFunction) {
    auto node = Ptr<Node>(new Decl());
    // 设置其他必要字段
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetFuncDecl(node, result, tokens, sourceManager);

    // 验证结果
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::FUNCTION_H);
}

// 测试 GetFuncDecl 函数 - 主构造函数
TEST_F(SemanticHighlightImplTest, GetFuncDecl_PrimaryConstructor) {
    Ptr<Node> node = Ptr<FuncDecl>();
    node->EnableAttr(Cangjie::AST::Attribute::PRIMARY_CONSTRUCTOR);
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 5};

    GetFuncDecl(Ptr<Node>(node), result, tokens, sourceManager);

    // 主构造函数不应产生高亮token
    EXPECT_TRUE(result.empty());
}

// 测试 GetFuncDecl 函数 - 无效标识符
TEST_F(SemanticHighlightImplTest, GetFuncDecl_InvalidIdentifier) {
    auto node = Ptr<FuncDecl>();
    node->identifier = "<invalid identifier>";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetFuncDecl(node, result, tokens, sourceManager);

    // 无效标识符不应产生高亮token
    EXPECT_TRUE(result.empty());
}

// 测试 GetPrimaryDecl 函数
TEST_F(SemanticHighlightImplTest, GetPrimaryDecl_NormalCase) {
    auto node = Ptr<PrimaryCtorDecl>();
    node->identifier = "PrimaryType";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetPrimaryDecl(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// 测试 GetVarDecl 函数
TEST_F(SemanticHighlightImplTest, GetVarDecl_NormalCase) {
    auto node = Ptr<VarDecl>();
    node->identifier = "testVariable";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetVarDecl(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

// 测试 GetPropDecl 函数
TEST_F(SemanticHighlightImplTest, GetPropDecl_NormalCase) {
    auto node = Ptr<PropDecl>();
    node->identifier = "testProperty";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetPropDecl(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

// 测试 GetCallExpr 函数
TEST_F(SemanticHighlightImplTest, GetCallExpr_NormalCase) {
    auto node = Ptr<CallExpr>();
    // 设置必要的字段
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

    // 验证结果
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::FUNCTION_H);
}

// 测试 GetCallExpr 函数 - MemberAccess基函数
TEST_F(SemanticHighlightImplTest, GetCallExpr_MemberAccessBase) {
    auto node = Ptr<CallExpr>();
    // 设置必要的字段
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

    // MemberAccess作为基函数时不应产生高亮token
    EXPECT_TRUE(result.empty());
}

// 测试 GetMemberAccess 函数
TEST_F(SemanticHighlightImplTest, GetMemberAccess_ClassMember) {
    auto node = Ptr<MemberAccess>();
    node->field = "memberField";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    // 设置 target 为 ClassLikeDecl
    auto target = Ptr<ClassDecl>();
    target->identifier = "TestClass";
    node->target = target;

    GetMemberAccess(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// 测试 GetMemberAccess 函数 - 包成员
TEST_F(SemanticHighlightImplTest, GetMemberAccess_PackageMember) {
    auto node = Ptr<MemberAccess>();
    node->field = "packageName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    // 设置 target 为 PackageDecl
    auto target = Ptr<PackageDecl>();
    node->target = target;

    GetMemberAccess(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::PACKAGE_H);
}

// 测试 GetFuncArg 函数
TEST_F(SemanticHighlightImplTest, GetFuncArg_NormalCase) {
    auto node = Ptr<FuncArg>();
    node->name = "argName";
    // 设置位置信息
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetFuncArg(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

// 测试 GetFuncArg 函数 - 零位置
TEST_F(SemanticHighlightImplTest, GetFuncArg_ZeroPosition) {
    auto node = Ptr<FuncArg>();
    node->name = "argName";
    // 设置零位置信息
    node->begin = Position{0, 0, 0};
    node->end = Position{0, 0, 0};

    GetFuncArg(node, result, tokens, sourceManager);

    // 零位置不应产生高亮token
    EXPECT_TRUE(result.empty());
}

// 测试 RefTargetEmpty 函数
TEST_F(SemanticHighlightImplTest, RefTargetEmpty_NullTarget) {
    auto node = Ptr<RefExpr>();
    // 不设置 ref.target

    bool isEmpty = RefTargetEmpty(node);

    EXPECT_TRUE(isEmpty);
}

// 测试 RefTargetEmpty 函数 - 非RefExpr节点
TEST_F(SemanticHighlightImplTest, RefTargetEmpty_NonRefExpr) {
    auto node = Ptr<Decl>();
    // 传入非RefExpr节点

    bool isEmpty = RefTargetEmpty(node);

    EXPECT_TRUE(isEmpty);
}

// 测试 SpecialTarget 函数
TEST_F(SemanticHighlightImplTest, SpecialTarget_ClassTarget) {
    auto node = Ptr<RefExpr>();
    auto target = Ptr<ClassDecl>();
    node->ref.target = target;

    bool isSpecial = SpecialTarget(node);

    EXPECT_TRUE(isSpecial);
}

// 测试 SpecialTarget 函数 - Init函数
TEST_F(SemanticHighlightImplTest, SpecialTarget_InitFunction) {
    auto node = Ptr<RefExpr>();
    auto target = Ptr<FuncDecl>();
    target->identifier = "init";
    node->ref.target = target;

    bool isSpecial = SpecialTarget(node);

    EXPECT_TRUE(isSpecial);
}

// 测试 HandleInterpolationExpr 函数
TEST_F(SemanticHighlightImplTest, HandleInterpolationExpr_DollarSign) {
    // 重新创建一个包含$的源码管理器
    delete sourceManager;
    sourceManager = new Cangjie::SourceManager();
    std::string testCode = "$variable";
    sourceManager->AddSource("test.cj", testCode);

    ark::Range range;
    range.start = Position{1, 1, 1};
    range.end = Position{1, 1, 10};

    HandleInterpolationExpr(range, sourceManager);

    // 验证范围是否正确调整（应偏移1个字符）
    EXPECT_EQ(range.start.column, 2);
    EXPECT_EQ(range.end.column, 11);
}

// 测试 HandleInterpolationExpr 函数 - 无源码管理器
TEST_F(SemanticHighlightImplTest, HandleInterpolationExpr_NoSourceManager) {
    ark::Range range;
    range.start = Position{1, 1, 1};
    range.end = Position{1, 1, 10};

    HandleInterpolationExpr(range, nullptr);

    // 应无变化
    EXPECT_EQ(range.start.column, 1);
    EXPECT_EQ(range.end.column, 10);
}

// 测试 GetRefExpr 函数
TEST_F(SemanticHighlightImplTest, GetRefExpr_ClassReference) {
    auto node = Ptr<RefExpr>();
    auto target = Ptr<ClassDecl>();
    target->identifier = "TestClass";
    node->ref.target = target;
    // 设置其他必要字段
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetRefExpr(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// 测试 GetRefExpr 函数 - 函数引用
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

// 测试 GetRefExpr 函数 - Init函数引用
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

// 测试 GetClassDecl 函数
TEST_F(SemanticHighlightImplTest, GetClassDecl_NormalCase) {
    auto node = Ptr<ClassDecl>();
    node->identifier = "TestClass";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetClassDecl(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// 测试 GetClassDecl 函数 - 无效标识符
TEST_F(SemanticHighlightImplTest, GetClassDecl_InvalidIdentifier) {
    auto node = Ptr<ClassDecl>();
    node->identifier = "<invalid identifier>";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetClassDecl(node, result, tokens, sourceManager);

    // 无效标识符不应产生高亮token
    EXPECT_TRUE(result.empty());
}

// 测试 GetRefType 函数
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

// 测试 GetRefType 函数 - 接口类型
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

// 测试 GetFuncParam 函数
TEST_F(SemanticHighlightImplTest, GetFuncParam_NormalCase) {
    auto node = Ptr<FuncParam>();
    node->identifier = "paramName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};
    // 不设置 isIdentifierCompilerAdd 或设置为 false

    GetFuncParam(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

// 测试 GetFuncParam 函数 - 编译器添加的标识符
TEST_F(SemanticHighlightImplTest, GetFuncParam_CompilerAdded) {
    auto node = Ptr<FuncParam>();
    node->identifier = "paramName";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};
    node->isIdentifierCompilerAdd = true;

    GetFuncParam(node, result, tokens, sourceManager);

    // 编译器添加的标识符不应产生高亮token
    EXPECT_TRUE(result.empty());
}

// 测试 GetInterfaceDecl 函数
TEST_F(SemanticHighlightImplTest, GetInterfaceDecl_NormalCase) {
    auto node = Ptr<InterfaceDecl>();
    node->identifier = "TestInterface";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 15};

    GetInterfaceDecl(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::INTERFACE_H);
}

// 测试 GetStructDecl 函数
TEST_F(SemanticHighlightImplTest, GetStructDecl_NormalCase) {
    auto node = Ptr<StructDecl>();
    node->identifier = "TestStruct";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 12};

    GetStructDecl(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// 测试 GetEnumDecl 函数
TEST_F(SemanticHighlightImplTest, GetEnumDecl_NormalCase) {
    auto node = Ptr<EnumDecl>();
    node->identifier = "TestEnum";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetEnumDecl(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// 测试 GetGenericParam 函数
TEST_F(SemanticHighlightImplTest, GetGenericParam_NormalCase) {
    auto node = Ptr<GenericParamDecl>();
    node->identifier = "T";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 2};

    GetGenericParam(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::VARIABLE_H);
}

// 测试 GetGenericParam 函数 - 无效标识符
TEST_F(SemanticHighlightImplTest, GetGenericParam_InvalidIdentifier) {
    auto node = Ptr<GenericParamDecl>();
    node->identifier = "<invalid identifier>";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetGenericParam(node, result, tokens, sourceManager);

    // 无效标识符不应产生高亮token
    EXPECT_TRUE(result.empty());
}

// 测试 GetQualifiedType 函数
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

// 测试 GetQualifiedType 函数 - 类类型
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

// 测试 GetTypeAliasDecl 函数
TEST_F(SemanticHighlightImplTest, GetTypeAliasDecl_NormalCase) {
    auto node = Ptr<TypeAliasDecl>();
    node->identifier = "TypeAlias";
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    GetTypeAliasDecl(node, result, tokens, sourceManager);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].kind, HighlightKind::CLASS_H);
}

// 测试 FindCharKeyWord 函数
TEST_F(SemanticHighlightImplTest, FindCharKeyWord_Keyword) {
    bool found = FindCharKeyWord("if");

    EXPECT_TRUE(found);
}

// 测试 FindCharKeyWord 函数 - 非关键字
TEST_F(SemanticHighlightImplTest, FindCharKeyWord_NonKeyword) {
    bool found = FindCharKeyWord("customIdentifier");

    EXPECT_FALSE(found);
}

// 测试 FindCharKeyWord 函数 - 关键字标识符
TEST_F(SemanticHighlightImplTest, FindCharKeyWord_KeywordIdentifier) {
    bool found = FindCharKeyWord("public");

    EXPECT_FALSE(found); // public在KEYWORD_IDENTIFIER中，应返回false
}

// 测试 SemanticHighlightImpl::NodeValid 函数
TEST_F(SemanticHighlightImplTest, NodeValid_ValidNode) {
    auto node = Ptr<Decl>();
    // 设置正确的文件ID和位置
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    bool valid = SemanticHighlightImpl::NodeValid(node, 1, "validName");

    EXPECT_TRUE(valid);
}

// 测试 SemanticHighlightImpl::NodeValid 函数 - 错误文件ID
TEST_F(SemanticHighlightImplTest, NodeValid_WrongFileId) {
    auto node = Ptr<Decl>();
    // 设置错误的文件ID
    node->begin = Position{2, 1, 1}; // 文件ID为2
    node->end = Position{2, 1, 10};

    bool valid = SemanticHighlightImpl::NodeValid(node, 1, "validName"); // 期望文件ID为1

    EXPECT_FALSE(valid);
}

// 测试 SemanticHighlightImpl::NodeValid 函数 - 关键字名称
TEST_F(SemanticHighlightImplTest, NodeValid_KeywordName) {
    auto node = Ptr<Decl>();
    node->begin = Position{1, 1, 1};
    node->end = Position{1, 1, 10};

    bool valid = SemanticHighlightImpl::NodeValid(node, 1, "if"); // if是关键字

    EXPECT_FALSE(valid);
}

// 测试 SemanticHighlightImpl::NodeValid 函数 - 零位置
TEST_F(SemanticHighlightImplTest, NodeValid_ZeroPosition) {
    auto node = Ptr<Decl>();
    // 不设置位置或设置为零位置
    node->begin = Position{0, 0, 0};
    node->end = Position{0, 0, 0};

    bool valid = SemanticHighlightImpl::NodeValid(node, 1, "validName");

    EXPECT_FALSE(valid);
}
