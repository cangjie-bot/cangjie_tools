// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "FindDeclUsage.cpp"
#include <unordered_set>

using namespace ark;
using namespace Cangjie::AST;

// CheckTypeEqual test
TEST(FindDeclUsageTest, CheckTypeEqual_SameBasicTypes) {
    // Create two identical basic types
    auto srcType = new PrimitiveTy(TypeKind::TYPE_INT32);

    auto targetType = new PrimitiveTy(TypeKind::TYPE_INT32);

    EXPECT_TRUE(CheckTypeEqual(*srcType, *targetType));
}

TEST(FindDeclUsageTest, CheckTypeEqual_DifferentBasicTypes) {
    // Create two different basic types
    auto srcType = new PrimitiveTy(TypeKind::TYPE_INT32);;

    auto targetType = new PrimitiveTy(TypeKind::TYPE_INT64);;

    EXPECT_FALSE(CheckTypeEqual(*srcType, *targetType));
}

TEST(FindDeclUsageTest, CheckTypeEqual_ArrayTypesWithDifferentDims) {
    // Create two array types with different dimensions
    auto elemTy= new PrimitiveTy(TypeKind::TYPE_INT32);
    auto srcArrayTy = std::make_unique<ArrayTy>(elemTy, 1);
    auto targetArrayTy = std::make_unique<ArrayTy>(elemTy, 2);

    EXPECT_FALSE(CheckTypeEqual(*srcArrayTy, *targetArrayTy));
}

TEST(FindDeclUsageTest, CheckTypeEqual_ArrayTypesWithSameDims) {
    // Create two array types with same dimensions
    auto elemTy= new PrimitiveTy(TypeKind::TYPE_INT32);
    auto srcArrayTy = std::make_unique<ArrayTy>(elemTy, 1);
    auto targetArrayTy = std::make_unique<ArrayTy>(elemTy, 1);

    EXPECT_TRUE(CheckTypeEqual(*srcArrayTy, *targetArrayTy));
}

// CheckParamListEqual test
TEST(FindDeclUsageTest, CheckParamListEqual_SameParamLists) {
    // Create two identical parameter lists
    auto srcList = new FuncParamList();
    auto targetList = new FuncParamList();

    // Add identical parameters
    auto param1 = new FuncParam();
    param1->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    auto param2 = new FuncParam();
    param2->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    srcList->params.emplace_back(param1);
    targetList->params.emplace_back(param2);

    EXPECT_TRUE(CheckParamListEqual(*srcList, *targetList));
}

TEST(FindDeclUsageTest, CheckParamListEqual_DifferentParamCount) {
    // Create two parameter lists with different parameter counts
    auto srcList = new FuncParamList();
    auto targetList = new FuncParamList();

    auto param1 = new FuncParam();
    param1->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    auto param2 = new FuncParam();
    param2->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    auto param3 = new FuncParam();
    param3->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    srcList->params.emplace_back(param1);
    targetList->params.emplace_back(param2);
    targetList->params.emplace_back(param3);

    EXPECT_FALSE(CheckParamListEqual(*srcList, *targetList));
}

// CheckFunctionEqual test
TEST(FindDeclUsageTest, CheckFunctionEqual_BothFunctionsNullBody) {
    // Create two function declarations without function bodies
    FuncDecl srcFunc;
    FuncDecl targetFunc;

    // No function body should return false
    EXPECT_FALSE(CheckFunctionEqual(srcFunc, targetFunc));
}

TEST(FindDeclUsageTest, CheckFunctionEqual_DifferentParamListCount) {
    // Create two function declarations with different parameter list counts
    FuncDecl srcFunc;
    FuncDecl targetFunc;

    srcFunc.funcBody = OwnedPtr<FuncBody>(new FuncBody());
    targetFunc.funcBody = OwnedPtr<FuncBody>(new FuncBody());

    // Add different parameter list counts
    auto paramList1 = Ptr<FuncParamList>(new FuncParamList());
    auto paramList2 = Ptr<FuncParamList>(new FuncParamList());
    auto paramList3 = Ptr<FuncParamList>(new FuncParamList());

    srcFunc.funcBody->paramLists.emplace_back(paramList1);
    targetFunc.funcBody->paramLists.emplace_back(paramList2);
    targetFunc.funcBody->paramLists.emplace_back(paramList3);

    EXPECT_FALSE(CheckFunctionEqual(srcFunc, targetFunc));
}

// GetDefinedDecl test
TEST(FindDeclUsageTest, GetDefinedDecl_FuncDeclWithPropDecl) {
    // Create function declaration with propDecl
    FuncDecl funcDecl;
    auto propDecl = Ptr<PropDecl>(new PropDecl());
    funcDecl.propDecl = propDecl;

    auto result = GetDefinedDecl(Ptr<const Decl>(&funcDecl));
    EXPECT_EQ(propDecl.get(), result.get());
}

TEST(FindDeclUsageTest, GetDefinedDecl_FuncDeclWithoutPropDecl) {
    // Create function declaration without propDecl
    FuncDecl funcDecl;

    auto result = GetDefinedDecl(Ptr<const Decl>(&funcDecl));
    EXPECT_EQ(&funcDecl, result.get());
}

// CheckDeclEqual test
TEST(FindDeclUsageTest, CheckDeclEqual_SameTypeDecls) {
    // Create two identical type declarations
    ClassDecl srcDecl;
    ClassDecl targetDecl;

    srcDecl.fullPackageName = "test.package";
    srcDecl.identifier = "TestClass";

    targetDecl.fullPackageName = "test.package";
    targetDecl.identifier = "TestClass";

    // Type declarations should return true
    EXPECT_TRUE(CheckDeclEqual(srcDecl, targetDecl));
}

TEST(FindDeclUsageTest, CheckDeclEqual_DifferentPackageNames) {
    // Create two declarations with different package names
    ClassDecl srcDecl;
    ClassDecl targetDecl;

    srcDecl.fullPackageName = "test.package1";
    srcDecl.identifier = "TestClass";

    targetDecl.fullPackageName = "test.package2";
    targetDecl.identifier = "TestClass";

    EXPECT_FALSE(CheckDeclEqual(srcDecl, targetDecl));
}

TEST(FindDeclUsageTest, CheckDeclEqual_DifferentIdentifiers) {
    // Create two declarations with different identifiers
    ClassDecl srcDecl;
    ClassDecl targetDecl;

    srcDecl.fullPackageName = "test.package";
    srcDecl.identifier = "TestClass1";

    targetDecl.fullPackageName = "test.package";
    targetDecl.identifier = "TestClass2";

    EXPECT_FALSE(CheckDeclEqual(srcDecl, targetDecl));
}

TEST(FindDeclUsageTest, CheckDeclEqual_NonFunctionDeclsSameContext) {
    // Create two non-function declarations with same context
    VarDecl srcDecl;
    VarDecl targetDecl;

    srcDecl.fullPackageName = "test.package";
    srcDecl.identifier = "testVar";

    targetDecl.fullPackageName = "test.package";
    targetDecl.identifier = "testVar";

    EXPECT_TRUE(CheckDeclEqual(srcDecl, targetDecl));
}

// GetRealNode test
TEST(FindDeclUsageTest, GetRealNode_ExprWithSourceExpr) {
    // Create expression with sourceExpr
    auto expr = Ptr<RefExpr>(new RefExpr());
    auto sourceExpr = Ptr<RefExpr>(new RefExpr());
    expr->sourceExpr = sourceExpr;

    auto result = GetRealNode(expr);
    EXPECT_EQ(sourceExpr.get(), result.get());
}

TEST(FindDeclUsageTest, GetRealNode_MemberAccessWithBuiltinOperator) {
    // Create member access expression with builtin operator
    auto ma = Ptr<MemberAccess>(new MemberAccess());
    // Assume "+" is one of the builtin operators
    ma->field = "+";

    auto callExpr = Ptr<CallExpr>(new CallExpr());
    auto sourceExpr = Ptr<RefExpr>(new RefExpr());
    callExpr->sourceExpr = sourceExpr;
    ma->callOrPattern = callExpr;

    auto result = GetRealNode(ma);
    EXPECT_EQ(sourceExpr.get(), result.get());
}

TEST(FindDeclUsageTest, GetRealNode_NormalNode) {
    // Create normal node
    auto node = Ptr<Decl>();

    auto result = GetRealNode(node);
    EXPECT_EQ(node.get(), result.get());
}

// checkMacroFunc test
TEST(FindDeclUsageTest, CheckMacroFunc_ValidMacroFunction) {
    // Create valid macro function
    Decl decl;
    auto target = Ptr<FuncDecl>(new FuncDecl());

    decl.isInMacroCall = true;
    decl.ty = Ptr<Ty>(new PrimitiveTy(TypeKind::TYPE_INT32));
    decl.identifier = "testFunc";

    target->ty = decl.ty; // Same type
    target->identifier = "testFunc";

    EXPECT_TRUE(checkMacroFunc(decl, target));
}

TEST(FindDeclUsageTest, CheckMacroFunc_NotInMacroCall) {
    // Create non-macro function
    Decl decl;
    auto target = Ptr<FuncDecl>(new FuncDecl());

    decl.isInMacroCall = false; // Not in macro call
    decl.ty = Ptr<Ty>();
    decl.identifier = "testFunc";

    target->ty = decl.ty;
    target->identifier = "testFunc";

    EXPECT_FALSE(checkMacroFunc(decl, target));
}

TEST(FindDeclUsageTest, CheckMacroFunc_DifferentIdentifiers) {
    // Create macro function with different identifiers
    Decl decl;
    auto target = Ptr<FuncDecl>(new FuncDecl());

    decl.isInMacroCall = true;
    decl.ty = Ptr<Ty>(new PrimitiveTy(TypeKind::TYPE_INT32));
    decl.identifier = "testFunc1";

    target->ty = Ptr<Ty>(new PrimitiveTy(TypeKind::TYPE_INT32));
    target->identifier = "testFunc2"; // Different identifier

    EXPECT_FALSE(checkMacroFunc(decl, target));
}

// FindNamedFuncParamUsage test
TEST(FindDeclUsageTest, FindNamedFuncParamUsage_NotNamedParam) {
    // Create non-named parameter
    FuncParam fp;
    fp.isNamedParam = false; // Non-named parameter

    Node root;
    auto result = FindNamedFuncParamUsage(fp, root);

    // Should directly return empty result set
    EXPECT_TRUE(result.empty());
}

TEST(FindDeclUsageTest, FindNamedFuncParamUsage_NoOuterDecl) {
    // Create named parameter without outer declaration
    FuncParam fp;
    fp.isNamedParam = true;
    fp.outerDecl = nullptr; // No outer declaration

    Node root;
    auto result = FindNamedFuncParamUsage(fp, root);

    // Should directly return empty result set
    EXPECT_TRUE(result.empty());
}

TEST(FindDeclUsageTest, FindNamedFuncParamUsage_OuterDeclNotFunc) {
    // Create named parameter with non-function outer declaration
    FuncParam fp;
    fp.isNamedParam = true;
    fp.outerDecl = Ptr<VarDecl>(); // Outer declaration is variable declaration

    Node root;
    auto result = FindNamedFuncParamUsage(fp, root);

    // Should directly return empty result set
    EXPECT_TRUE(result.empty());
}

// FindDeclUsage test
TEST(FindDeclUsageTest, FindDeclUsage_FuncParamDecl) {
    // Create function parameter declaration
    FuncParam fp;
    fp.isNamedParam = true;

    Node root;
    auto result = FindDeclUsage(fp, root, false);

    // Should call FindNamedFuncParamUsage
    EXPECT_TRUE(result.empty()); // Should be empty in this simple test
}

TEST(FindDeclUsageTest, FindDeclUsage_NormalDecl) {
    // Create normal declaration
    ClassDecl decl;
    decl.identifier = "TestClass";

    Node root;
    auto result = FindDeclUsage(decl, root, false);

    // Should call FindUsage
    EXPECT_TRUE(result.empty()); // Should be empty in this simple test
}
