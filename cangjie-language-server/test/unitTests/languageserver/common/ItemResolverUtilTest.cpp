// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "ItemResolverUtil.cpp"
#include <string>

using namespace ark;
using namespace Cangjie::AST;

// Test ResolveNameByNode function
TEST(ItemResolverUtilTest, ResolveNameByNode_MacroExpandDeclWithInvocation) {
    // Create MacroExpandDecl node with invocation declaration
    auto macroExpandDecl = Ptr<MacroExpandDecl>();
    auto invocationDecl = OwnedPtr<Decl>();
    invocationDecl->identifier = "testFunction";
    macroExpandDecl->invocation.decl = std::move(invocationDecl);

    std::string result = ItemResolverUtil::ResolveNameByNode(*macroExpandDecl);
    EXPECT_EQ("testFunction", result);
}

TEST(ItemResolverUtilTest, ResolveNameByNode_MacroExpandDeclWithoutInvocation) {
    // Create MacroExpandDecl node without invocation declaration
    auto macroExpandDecl = Ptr<MacroExpandDecl>();
    macroExpandDecl->invocation.decl = nullptr;

    std::string result = ItemResolverUtil::ResolveNameByNode(*macroExpandDecl);
    EXPECT_EQ("", result);
}

TEST(ItemResolverUtilTest, ResolveNameByNode_DeclNode) {
    // Create Decl node
    auto decl = Ptr<FuncDecl>();
    decl->identifier = "testFunction";

    std::string result = ItemResolverUtil::ResolveNameByNode(*decl);
    EXPECT_EQ("testFunction", result);
}

TEST(ItemResolverUtilTest, ResolveNameByNode_PackageNode) {
    // Create Package node
    auto package = Ptr<Package>();
    package->fullPackageName = "test.package";

    std::string result = ItemResolverUtil::ResolveNameByNode(*package);
    EXPECT_EQ("test.package", result);
}

TEST(ItemResolverUtilTest, ResolveNameByNode_UnknownNode) {
    // Create unknown type node
    auto node = Ptr<Node>();

    std::string result = ItemResolverUtil::ResolveNameByNode(*node);
    EXPECT_EQ("", result);
}

// Test ResolveKindByNode function
TEST(ItemResolverUtilTest, ResolveKindByNode_VarDecl) {
    // Create VarDecl node
    auto varDecl = Ptr<VarDecl>();

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*varDecl);
    EXPECT_EQ(CompletionItemKind::CIK_VARIABLE, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_FuncDeclConstructor) {
    // Create constructor FuncDecl node
    auto funcDecl = Ptr<FuncDecl>();
    funcDecl->EnableAttr(Attribute::CONSTRUCTOR);

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*funcDecl);
    EXPECT_EQ(CompletionItemKind::CIK_CONSTRUCTOR, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_FuncDeclMethod) {
    // Create normal function FuncDecl node
    auto funcDecl = Ptr<FuncDecl>();

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*funcDecl);
    EXPECT_EQ(CompletionItemKind::CIK_METHOD, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_ClassDecl) {
    // Create ClassDecl node
    auto classDecl = Ptr<ClassDecl>();

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*classDecl);
    EXPECT_EQ(CompletionItemKind::CIK_CLASS, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_InterfaceDecl) {
    // Create InterfaceDecl node
    auto interfaceDecl = Ptr<InterfaceDecl>();

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*interfaceDecl);
    EXPECT_EQ(CompletionItemKind::CIK_INTERFACE, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_EnumDecl) {
    // Create EnumDecl node
    auto enumDecl = Ptr<EnumDecl>();

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*enumDecl);
    EXPECT_EQ(CompletionItemKind::CIK_ENUM, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_StructDecl) {
    // Create StructDecl node
    auto structDecl = Ptr<StructDecl>();

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*structDecl);
    EXPECT_EQ(CompletionItemKind::CIK_STRUCT, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_TypeAliasDecl) {
    // Create TypeAliasDecl node
    auto typeAliasDecl = Ptr<TypeAliasDecl>();

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*typeAliasDecl);
    EXPECT_EQ(CompletionItemKind::CIK_CLASS, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_MacroExpandDeclWithInvocation) {
    // Create MacroExpandDecl node with invocation declaration
    auto macroExpandDecl = Ptr<MacroExpandDecl>();
    auto invocationDecl = OwnedPtr<Decl>();
    macroExpandDecl->invocation.decl = std::move(invocationDecl);

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*macroExpandDecl);
    EXPECT_EQ(CompletionItemKind::CIK_METHOD, result);
}

TEST(ItemResolverUtilTest, ResolveKindByNode_MacroExpandDeclWithoutInvocation) {
    // Create MacroExpandDecl node without invocation declaration
    auto macroExpandDecl = Ptr<MacroExpandDecl>();
    macroExpandDecl->invocation.decl = nullptr;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByNode(*macroExpandDecl);
    EXPECT_EQ(CompletionItemKind::CIK_MISSING, result);
}

// Test ResolveKindByASTKind function
TEST(ItemResolverUtilTest, ResolveKindByASTKind_VarDecl) {
    // Test VAR_DECL type
    ASTKind astKind = ASTKind::VAR_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_VARIABLE, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_FuncDecl) {
    // Test FUNC_DECL type
    ASTKind astKind = ASTKind::FUNC_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_METHOD, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_ClassDecl) {
    // Test CLASS_DECL type
    ASTKind astKind = ASTKind::CLASS_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_CLASS, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_InterfaceDecl) {
    // Test INTERFACE_DECL type
    ASTKind astKind = ASTKind::INTERFACE_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_INTERFACE, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_EnumDecl) {
    // Test ENUM_DECL type
    ASTKind astKind = ASTKind::ENUM_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_ENUM, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_StructDecl) {
    // Test STRUCT_DECL type
    ASTKind astKind = ASTKind::STRUCT_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_STRUCT, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_TypeAliasDecl) {
    // Test TYPE_ALIAS_DECL type
    ASTKind astKind = ASTKind::TYPE_ALIAS_DECL;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_CLASS, result);
}

TEST(ItemResolverUtilTest, ResolveKindByASTKind_Unknown) {
    // Test unknown type
    ASTKind astKind = ASTKind::NODE;

    CompletionItemKind result = ItemResolverUtil::ResolveKindByASTKind(astKind);
    EXPECT_EQ(CompletionItemKind::CIK_MISSING, result);
}

// Test GetGenericParamByDecl function
TEST(ItemResolverUtilTest, GetGenericParamByDecl_NullGeneric) {
    // Test null Generic declaration
    Ptr<Generic> genericDecl = nullptr;

    std::string result = ItemResolverUtil::GetGenericParamByDecl(genericDecl);
    EXPECT_EQ("", result);
}

TEST(ItemResolverUtilTest, GetGenericParamByDecl_EmptyGeneric) {
    // Test empty Generic declaration (no type parameters)
    auto genericDecl = Ptr<Generic>();

    std::string result = ItemResolverUtil::GetGenericParamByDecl(genericDecl);
    EXPECT_EQ("<>", result);
}

TEST(ItemResolverUtilTest, GetGenericParamByDecl_SingleGenericParam) {
    // Test Generic declaration with single type parameter
    auto genericDecl = Ptr<Generic>();
    auto param = Ptr<GenericParamDecl>();
    param->identifier = "T";
    genericDecl->typeParameters.emplace_back(param);

    std::string result = ItemResolverUtil::GetGenericParamByDecl(genericDecl);
    EXPECT_EQ("<T>", result);
}

TEST(ItemResolverUtilTest, GetGenericParamByDecl_MultipleGenericParams) {
    // Test Generic declaration with multiple type parameters
    auto genericDecl = Ptr<Generic>();
    auto param1 = Ptr<GenericParamDecl>();
    param1->identifier = "T";
    auto param2 = Ptr<GenericParamDecl>();
    param2->identifier = "U";
    genericDecl->typeParameters.emplace_back(param1);
    genericDecl->typeParameters.emplace_back(param2);

    std::string result = ItemResolverUtil::GetGenericParamByDecl(genericDecl);
    EXPECT_EQ("<T, U>", result);
}

// Test ResolveSignatureByNode function
TEST(ItemResolverUtilTest, ResolveSignatureByNode_VarDecl) {
    // Test signature resolution for VarDecl node
    auto varDecl = Ptr<VarDecl>();
    varDecl->identifier = "testVar";

    std::string result = ItemResolverUtil::ResolveSignatureByNode(*varDecl, nullptr);
    EXPECT_EQ("testVar", result);
}

TEST(ItemResolverUtilTest, ResolveSignatureByNode_FuncDecl) {
    // Test signature resolution for FuncDecl node
    auto funcDecl = Ptr<FuncDecl>();
    funcDecl->identifier = "testFunction";

    std::string result = ItemResolverUtil::ResolveSignatureByNode(*funcDecl, nullptr);
    EXPECT_EQ("testFunction", result);
}

TEST(ItemResolverUtilTest, ResolveSignatureByNode_ClassDecl) {
    // Test signature resolution for ClassDecl node
    auto classDecl = Ptr<ClassDecl>();
    classDecl->identifier = "TestClass";

    std::string result = ItemResolverUtil::ResolveSignatureByNode(*classDecl, nullptr);
    EXPECT_EQ("TestClass", result);
}

TEST(ItemResolverUtilTest, ResolveSignatureByNode_ClassDeclWithGeneric) {
    // Test signature resolution for ClassDecl node with generic parameters
    auto classDecl = Ptr<ClassDecl>();
    classDecl->identifier = "TestClass";
    auto generic = OwnedPtr<Generic>();
    auto param = Ptr<GenericParamDecl>();
    param->identifier = "T";
    generic->typeParameters.emplace_back(param);
    classDecl->generic = std::move(generic);

    std::string result = ItemResolverUtil::ResolveSignatureByNode(*classDecl, nullptr);
    EXPECT_EQ("TestClass<T>", result);
}

// Test IsCustomAnnotation function
TEST(ItemResolverUtilTest, IsCustomAnnotation_NoOuterDecl) {
    // Test case where there is no outer declaration
    FuncDecl decl;

    bool result = ItemResolverUtil::IsCustomAnnotation(decl);
    EXPECT_FALSE(result);
}

TEST(ItemResolverUtilTest, IsCustomAnnotation_NoAnnotations) {
    // Test case where outer declaration has no annotations
    FuncDecl decl;
    auto outerDecl = Ptr<FuncDecl>();
    decl.outerDecl = outerDecl;

    bool result = ItemResolverUtil::IsCustomAnnotation(decl);
    EXPECT_FALSE(result);
}

TEST(ItemResolverUtilTest, IsCustomAnnotation_WithCustomAnnotation) {
    // Test case with custom annotation
    FuncDecl decl;
    auto outerDecl = Ptr<FuncDecl>();
    auto annotation = Ptr<Annotation>(new Annotation("TestAnnotation", AnnotationKind::CUSTOM, Position{1, 1, 1}));
    outerDecl->annotations.emplace_back(annotation);
    decl.outerDecl = outerDecl;

    bool result = ItemResolverUtil::IsCustomAnnotation(decl);
    EXPECT_TRUE(result);
}
