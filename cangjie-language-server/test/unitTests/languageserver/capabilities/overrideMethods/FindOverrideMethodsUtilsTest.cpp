// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "FindOverrideMethodsUtils.h"

using namespace ark;
using namespace Cangjie::AST;

// Test for ResolveDeclModifiers function
TEST(FindOverrideMethodUtilsTest, ResolveDeclModifiersTest001) {
    auto decl = new FuncDecl();
    decl->EnableAttr(Attribute::PUBLIC);
    decl->EnableAttr(Attribute::STATIC);

    auto result = ResolveDeclModifiers(Ptr<Decl>(decl));
    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(std::find(result.begin(), result.end(), "public") != result.end());
    EXPECT_TRUE(std::find(result.begin(), result.end(), "static") != result.end());
}

TEST(FindOverrideMethodUtilsTest, ResolveDeclModifiersTest002) {
    auto decl = new FuncDecl();
    // No modifiers
    auto result = ResolveDeclModifiers(Ptr<Decl>(decl));
    EXPECT_TRUE(result.empty());
}

TEST(FindOverrideMethodUtilsTest, ResolveDeclModifiersTest003) {
    auto decl = new PropDecl();
    decl->EnableAttr(Attribute::PRIVATE);
    decl->EnableAttr(Attribute::PROTECTED);

    auto result = ResolveDeclModifiers(Ptr<Decl>(decl));
    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(std::find(result.begin(), result.end(), "private") != result.end());
    EXPECT_TRUE(std::find(result.begin(), result.end(), "protected") != result.end());
}

// Test for ResolveDeclIdentifier function
TEST(FindOverrideMethodUtilsTest, ResolveDeclIdentifierTest001) {
    auto decl = new FuncDecl();
    decl->identifier = "myFunction";

    auto result = ResolveDeclIdentifier(Ptr<Decl>(decl));
    EXPECT_EQ(result, "myFunction");
}

TEST(FindOverrideMethodUtilsTest, ResolveDeclIdentifierTest002) {
    auto decl = new PropDecl();
    decl->identifier = "myProperty";

    auto result = ResolveDeclIdentifier(Ptr<Decl>(decl));
    EXPECT_EQ(result, "myProperty");
}

TEST(FindOverrideMethodUtilsTest, ResolveDeclIdentifierTest003) {
    auto decl = new ClassDecl();
    decl->identifier = "";

    auto result = ResolveDeclIdentifier(Ptr<Decl>(decl));
    EXPECT_EQ(result, "");
}

// Test for ResolveFuncParamList function
TEST(FindOverrideMethodUtilsTest, ResolveFuncParamListTest001) {
    auto funcDecl = new FuncDecl();
    auto funcBody = new FuncBody();
    auto paramList = new FuncParamList();
    paramList->params.emplace_back(nullptr);
    funcBody->paramLists.emplace_back(paramList);
    funcDecl->funcBody = OwnedPtr<FuncBody>(funcBody);

    const FuncParamDetailList result = ResolveFuncParamList(Ptr<FuncDecl>(funcDecl));
    EXPECT_TRUE(result.params.empty());
}

TEST(FindOverrideMethodUtilsTest, ResolveFuncParamListTest002) {
    auto funcDecl = new FuncDecl();
    auto funcBody = new FuncBody();
    auto paramList = new FuncParamList();

    auto param = new FuncParam();
    param->identifier = "param1";
    param->ty = new PrimitiveTy(TypeKind::TYPE_INT32);
    paramList->params.emplace_back(param);
    funcBody->paramLists.emplace_back(paramList);
    funcDecl->funcBody = OwnedPtr<FuncBody>(funcBody);

    const FuncParamDetailList result = ResolveFuncParamList(Ptr<FuncDecl>(funcDecl));
    EXPECT_EQ(result.params.size(), 1);
    EXPECT_EQ(result.params[0].identifier, "param1");
    EXPECT_NE(result.params[0].type, nullptr);
}

TEST(FindOverrideMethodUtilsTest, ResolveFuncParamListTest003) {
    auto funcDecl = new FuncDecl();
    auto funcBody = new FuncBody();
    auto paramList = new FuncParamList();

    auto param1 = new FuncParam();
    param1->identifier = "param1";
    param1->ty = new PrimitiveTy(TypeKind::TYPE_INT32);
    param1->isNamedParam = true;

    auto param2 = new FuncParam();
    param2->identifier = "param2";
    param2->ty = new PrimitiveTy(TypeKind::TYPE_FLOAT32);
    paramList->variadicArgIndex = 1;

    paramList->params.emplace_back(param1);
    paramList->params.emplace_back(param2);
    funcBody->paramLists.emplace_back(paramList);
    funcDecl->funcBody = OwnedPtr<FuncBody>(funcBody);

    const FuncParamDetailList result = ResolveFuncParamList(Ptr<FuncDecl>(funcDecl));
    EXPECT_EQ(result.params.size(), 2);
    EXPECT_TRUE(result.params[0].isNamed);
    EXPECT_TRUE(result.isVariadic);
}

// Test for ResolveFuncRetType function
TEST(FindOverrideMethodUtilsTest, ResolveFuncRetTypeTest001) {
    auto funcDecl = new FuncDecl();
    auto funcBody = new FuncBody();
    auto retType = new Type();
    funcDecl->funcBody = OwnedPtr<FuncBody>(funcBody);
    funcBody->retType = OwnedPtr<Type>(retType);
    funcDecl->funcBody->retType->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    auto result = ResolveFuncRetType(Ptr<FuncDecl>(funcDecl));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->ToString(), "Int32");
}

TEST(FindOverrideMethodUtilsTest, ResolveFuncRetTypeTest002) {
    auto funcDecl = new FuncDecl();
    // No return type
    auto result = ResolveFuncRetType(Ptr<FuncDecl>(funcDecl));
    EXPECT_EQ(result, nullptr);
}

// Test for ResolveType function
TEST(FindOverrideMethodUtilsTest, ResolveTypeTest001) {
    // Test with ClassTy
    auto classDecl = new ClassDecl();
    classDecl->identifier = "MyClass";
    auto classTy = new ClassTy("MyClass", *classDecl, {});
    auto result = ResolveType(Ptr<Ty>(classTy));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->ToString(), "MyClass");
}

TEST(FindOverrideMethodUtilsTest, ResolveTypeTest002) {
    // Test with FuncTy
    auto retTy = new PrimitiveTy(TypeKind::TYPE_INT32);
    std::vector<Ptr<Ty>> paramTys;
    paramTys.push_back(Ptr<Ty>(new PrimitiveTy(TypeKind::TYPE_INT32)));

    auto funcTy = new FuncTy(paramTys, Ptr<Ty>(retTy));
    auto result = ResolveType(Ptr<Ty>(funcTy));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->ToString(), "(Int32) -> Int32");
}

TEST(FindOverrideMethodUtilsTest, ResolveTypeTest003) {
    // Test with nullptr
    auto result = ResolveType(nullptr);
    EXPECT_EQ(result, nullptr);
}


// Test for ResolveType function - InterfaceTy branch
TEST(FindOverrideMethodUtilsTest, ResolveTypeTest004) {
    // Test with InterfaceTy
    auto interfaceDecl = new InterfaceDecl();
    interfaceDecl->identifier = "MyInterface";
    auto interfaceTy = new InterfaceTy("MyInterface", *interfaceDecl, {});
    auto result = ResolveType(Ptr<Ty>(interfaceTy));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->ToString(), "MyInterface");
}

// Test for ResolveType function - EnumTy branch
TEST(FindOverrideMethodUtilsTest, ResolveTypeTest005) {
    // Test with EnumTy
    auto enumDecl = new EnumDecl();
    enumDecl->identifier = "MyEnum";
    auto enumTy = new EnumTy("MyEnum", *enumDecl, {});
    auto result = ResolveType(Ptr<Ty>(enumTy));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->ToString(), "MyEnum");
}

// Test for ResolveType function - StructTy branch
TEST(FindOverrideMethodUtilsTest, ResolveTypeTest006) {
    // Test with StructTy
    auto structDecl = new StructDecl();
    structDecl->identifier = "MyStruct";
    auto structTy = new StructTy("MyStruct", *structDecl, {});
    auto result = ResolveType(Ptr<Ty>(structTy));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->ToString(), "MyStruct");
}

// Test for ResolveType function - TypeAliasTy branch
TEST(FindOverrideMethodUtilsTest, ResolveTypeTest007) {
    // Test with TypeAliasTy
    auto typeAliasDecl = new TypeAliasDecl();
    typeAliasDecl->identifier = "MyAlias";
    auto typeAliasTy = new TypeAliasTy("MyAlias", *typeAliasDecl, {});
    auto result = ResolveType(Ptr<Ty>(typeAliasTy));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->ToString(), "MyAlias");
}

// Test for ResolveType function - GenericsTy branch
TEST(FindOverrideMethodUtilsTest, ResolveTypeTest008) {
    // Test with GenericsTy
    auto genericParamDecl = new GenericParamDecl();
    auto genericsTy = new GenericsTy("T", *genericParamDecl);
    auto result = ResolveType(Ptr<Ty>(genericsTy));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->ToString(), "T");
}

// Test for ResolveType function - VArrayTy branch
TEST(FindOverrideMethodUtilsTest, ResolveTypeTest009) {
    // Test with VArrayTy
    auto elemTy = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto varrayTy = new VArrayTy(Ptr<Ty>(elemTy), 10);
    auto result = ResolveType(Ptr<Ty>(varrayTy));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->ToString(), "VArray<Int32, $10>");
}

// Test for ResolveType function - TupleTy branch
TEST(FindOverrideMethodUtilsTest, ResolveTypeTest010) {
    // Test with TupleTy
    std::vector<Ptr<Ty>> elemTys;
    elemTys.push_back(Ptr<Ty>(new PrimitiveTy(TypeKind::TYPE_INT32)));
    elemTys.push_back(Ptr<Ty>(new PrimitiveTy(TypeKind::TYPE_FLOAT32)));
    auto tupleTy = new TupleTy(elemTys);
    auto result = ResolveType(Ptr<Ty>(tupleTy));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->ToString(), "(Int32, Float32)");
}

// Test for ResolveType function - default Ty branch
TEST(FindOverrideMethodUtilsTest, ResolveTypeTest011) {
    // Test with PrimitiveTy (default case)
    auto primitiveTy = new PrimitiveTy(TypeKind::TYPE_INT32);
    auto result = ResolveType(Ptr<Ty>(primitiveTy));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->ToString(), "Int32");
}

// Test for ResolveFuncDetail function
TEST(FindOverrideMethodUtilsTest, ResolveFuncDetailTest001) {
    auto funcDecl = new FuncDecl();
    funcDecl->EnableAttr(Attribute::PUBLIC);
    funcDecl->identifier = "myFunction";

    auto funcBody = new FuncBody();
    auto paramList = new FuncParamList();
    auto param = new FuncParam();
    param->identifier = "param1";
    param->ty = new PrimitiveTy(TypeKind::TYPE_INT32);
    paramList->params.emplace_back(param);
    funcBody->paramLists.emplace_back(paramList);

    funcBody->retType = OwnedPtr<Type>(new Type());
    funcBody->retType->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    funcDecl->funcBody = OwnedPtr<FuncBody>(funcBody);

    auto result = ResolveFuncDetail(Ptr<FuncDecl>(funcDecl));
    EXPECT_EQ(result.modifiers.size(), 1);
    EXPECT_EQ(result.identifier, "myFunction");
    EXPECT_EQ(result.params.params.size(), 1);
    EXPECT_NE(result.retType, nullptr);
    EXPECT_EQ(result.ToString(), "public func myFunction(param1: Int32): Int32");
}

TEST(FindOverrideMethodUtilsTest, ResolveFuncDetailTest002) {
    auto funcDecl = new FuncDecl();
    funcDecl->identifier = "emptyFunc";
    funcDecl->funcBody = OwnedPtr<FuncBody>(new FuncBody());

    auto result = ResolveFuncDetail(Ptr<FuncDecl>(funcDecl));
    EXPECT_TRUE(result.modifiers.empty());
    EXPECT_EQ(result.identifier, "emptyFunc");
    EXPECT_TRUE(result.params.params.empty());
    EXPECT_EQ(result.retType, nullptr);
}

// Test for ResolvePropDetail function
TEST(FindOverrideMethodUtilsTest, ResolvePropDetailTest001) {
    auto propDecl = new PropDecl();
    propDecl->EnableAttr(Attribute::PRIVATE);
    propDecl->identifier = "myProperty";
    propDecl->ty = new PrimitiveTy(TypeKind::TYPE_CSTRING);

    auto result = ResolvePropDetail(Ptr<PropDecl>(propDecl));
    EXPECT_EQ(result.modifiers.size(), 1);
    EXPECT_EQ(result.identifier, "myProperty");
    EXPECT_NE(result.type, nullptr);
    EXPECT_EQ(result.ToString(), "private prop myProperty: CString");
}

TEST(FindOverrideMethodUtilsTest, ResolvePropDetailTest002) {
    auto propDecl = new PropDecl();
    propDecl->identifier = "count";
    propDecl->ty = new PrimitiveTy(TypeKind::TYPE_INT32);

    auto result = ResolvePropDetail(Ptr<PropDecl>(propDecl));
    EXPECT_TRUE(result.modifiers.empty());
    EXPECT_EQ(result.identifier, "count");
    EXPECT_NE(result.type, nullptr);
}

// TypeDetail tests
TEST(TypeDetailTest, ToStringTest001) {
    TypeDetail detail("MyType");
    EXPECT_EQ(detail.ToString(), "MyType");
}

TEST(TypeDetailTest, ToStringTest002) {
    TypeDetail detail("");
    EXPECT_EQ(detail.ToString(), "");
}

TEST(TypeDetailTest, SetIdentifierTest001) {
    TypeDetail detail("OldType");
    detail.SetIdentifier("OldType", "NewType");
    EXPECT_EQ(detail.identifier, "OldType"); // Base class doesn't change
}

// CommonTypeDetail tests
TEST(CommonTypeDetailTest, ToStringTest001) {
    CommonTypeDetail detail("GenericType");
    EXPECT_EQ(detail.ToString(), "GenericType");
}

TEST(CommonTypeDetailTest, SetIdentifierTest001) {
    CommonTypeDetail detail("OldType");
    detail.SetIdentifier("OldType", "NewType");
    EXPECT_EQ(detail.identifier, "NewType");
}

TEST(CommonTypeDetailTest, SetIdentifierTest002) {
    CommonTypeDetail detail("OldType");
    detail.SetIdentifier("DifferentType", "NewType");
    EXPECT_EQ(detail.identifier, "OldType"); // Should not change
}

// ClassLikeTypeDetail tests
TEST(ClassLikeTypeDetailTest, ToStringTest001) {
    ClassLikeTypeDetail detail("MyClass");
    EXPECT_EQ(detail.ToString(), "MyClass");
}

TEST(ClassLikeTypeDetailTest, ToStringTest002) {
    ClassLikeTypeDetail detail("MyGenericClass");
    detail.generics.push_back(std::make_unique<CommonTypeDetail>("T"));
    detail.generics.push_back(std::make_unique<CommonTypeDetail>("U"));
    EXPECT_EQ(detail.ToString(), "MyGenericClass<T, U>");
}

TEST(ClassLikeTypeDetailTest, SetIdentifierTest001) {
    ClassLikeTypeDetail detail("Container");
    detail.generics.push_back(std::make_unique<CommonTypeDetail>("T"));
    detail.SetIdentifier("T", "NewType");
    EXPECT_EQ(detail.generics[0]->ToString(), "NewType");
}

// FuncLikeTypeDetail tests
TEST(FuncLikeTypeDetailTest, ToStringTest001) {
    FuncLikeTypeDetail detail;
    detail.params.push_back(std::make_unique<CommonTypeDetail>("int32"));
    detail.ret = std::make_unique<CommonTypeDetail>("void");
    EXPECT_EQ(detail.ToString(), "(int32) -> void");
}

TEST(FuncLikeTypeDetailTest, ToStringTest002) {
    FuncLikeTypeDetail detail;
    detail.params.push_back(std::make_unique<CommonTypeDetail>("string"));
    detail.params.push_back(std::make_unique<CommonTypeDetail>("bool"));
    detail.ret = std::make_unique<CommonTypeDetail>("Result");
    EXPECT_EQ(detail.ToString(), "(string, bool) -> Result");
}

TEST(FuncLikeTypeDetailTest, SetIdentifierTest001) {
    FuncLikeTypeDetail detail;
    detail.params.push_back(std::make_unique<CommonTypeDetail>("T"));
    detail.ret = std::make_unique<CommonTypeDetail>("T");
    detail.SetIdentifier("T", "ConcreteType");
    EXPECT_EQ(detail.params[0]->ToString(), "ConcreteType");
    EXPECT_EQ(detail.ret->ToString(), "ConcreteType");
}

// VArrayTypeDetail tests
TEST(VArrayTypeDetailTest, ToStringTest001) {
    VArrayTypeDetail detail;
    detail.identifier = "Array";
    detail.tyArg = std::make_unique<CommonTypeDetail>("int32");
    detail.size = 10;
    EXPECT_EQ(detail.ToString(), "Array<int32, $10>");
}

TEST(VArrayTypeDetailTest, ToStringTest002) {
    VArrayTypeDetail detail;
    detail.identifier = "Matrix";
    detail.tyArg = std::make_unique<CommonTypeDetail>("float64");
    detail.size = 0;
    EXPECT_EQ(detail.ToString(), "Matrix<float64, $0>");
}

// TupleTypeDetail tests
TEST(TupleTypeDetailTest, ToStringTest001) {
    TupleTypeDetail detail;
    detail.params.push_back(std::make_unique<CommonTypeDetail>("int32"));
    detail.params.push_back(std::make_unique<CommonTypeDetail>("string"));
    EXPECT_EQ(detail.ToString(), "(int32, string)");
}

TEST(TupleTypeDetailTest, ToStringTest002) {
    TupleTypeDetail detail;
    EXPECT_EQ(detail.ToString(), "()");
}

// FuncParamDetail tests
TEST(FuncParamDetailTest, ToStringTest001) {
    FuncParamDetail param;
    param.identifier = "value";
    param.type = std::make_unique<CommonTypeDetail>("int32");
    EXPECT_EQ(param.ToString(), "value: int32");
}

TEST(FuncParamDetailTest, ToStringTest002) {
    FuncParamDetail param;
    param.identifier = "name";
    param.isNamed = true;
    param.type = std::make_unique<CommonTypeDetail>("string");
    EXPECT_EQ(param.ToString(), "name!: string");
}

TEST(FuncParamDetailTest, SetGenericTypeTest001) {
    FuncParamDetail param;
    param.type = std::make_unique<CommonTypeDetail>("T");
    param.SetGenericType("T", "ConcreteType");
    EXPECT_EQ(param.type->ToString(), "ConcreteType");
}

TEST(FuncParamDetailListTest, ToStringTest001) {
    FuncParamDetailList list;
    FuncParamDetail param1;
    param1.identifier = "x";
    param1.type = std::make_unique<CommonTypeDetail>("int32");

    FuncParamDetail param2;
    param2.identifier = "y";
    param2.type = std::make_unique<CommonTypeDetail>("int32");

    list.params.push_back(std::move(param1));
    list.params.push_back(std::move(param2));
    EXPECT_EQ(list.ToString(), "x: int32, y: int32");
}

TEST(FuncParamDetailListTest, ToStringTest002) {
    FuncParamDetailList list;
    list.isVariadic = true;
    FuncParamDetail param;
    param.identifier = "items";
    param.type = std::make_unique<CommonTypeDetail>("string");
    list.params.push_back(std::move(param));
    EXPECT_EQ(list.ToString(), "items: string, ...");
}


// FuncDetail tests
TEST(FuncDetailTest, ToStringTest001) {
    FuncDetail detail;
    detail.modifiers = {"public", "static"};
    detail.identifier = "calculate";
    
    FuncParamDetail param;
    param.identifier = "value";
    param.type = std::make_unique<CommonTypeDetail>("int32");
    detail.params.params.push_back(std::move(param));

    detail.retType = std::make_unique<CommonTypeDetail>("float64");

    EXPECT_EQ(detail.ToString(), "public static func calculate(value: int32): float64");
}


TEST(FuncDetailTest, GetFunctionNameTest001) {
    FuncDetail detail;
    detail.identifier = "print";
    
    FuncParamDetail param;
    param.identifier = "message";
    param.type = std::make_unique<CommonTypeDetail>("string");
    detail.params.params.push_back(std::move(param));

    EXPECT_EQ(detail.GetFunctionName(), "print(message: string)");
}

TEST(FuncDetailTest, GetFunctionWithRetTest001) {
    FuncDetail detail;
    detail.identifier = "parse";

    FuncParamDetail param;
    param.identifier = "input";
    param.type = std::make_unique<CommonTypeDetail>("string");
    detail.params.params.push_back(std::move(param));

    detail.retType = std::make_unique<CommonTypeDetail>("Result");

    EXPECT_EQ(detail.GetFunctionWithRet(), "parse(input: string): Result");
}


// PropDetail tests
TEST(PropDetailTest, ToStringTest001) {
    PropDetail detail;
    detail.modifiers = {"private"};
    detail.identifier = "count";
    detail.type = std::make_unique<CommonTypeDetail>("int32");
    EXPECT_EQ(detail.ToString(), "private prop count: int32");
}

TEST(PropDetailTest, ToStringTest002) {
    PropDetail detail;
    detail.identifier = "name";
    detail.type = std::make_unique<CommonTypeDetail>("string");
    EXPECT_EQ(detail.ToString(), "prop name: string");
}

TEST(PropDetailTest, SetGenericTypeTest001) {
    PropDetail detail;
    detail.type = std::make_unique<CommonTypeDetail>("T");
    detail.SetGenericType("T", "ConcreteType");
    EXPECT_EQ(detail.type->ToString(), "ConcreteType");
}