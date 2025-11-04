// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "Utils.cpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

using namespace ark;

struct FakeFuncTy : public Cangjie::AST::FuncTy {
    FakeFuncTy(const std::vector<Ptr<Ty>> &params = {},
        Ptr<Ty> ret = nullptr,
        const Config &cfg = {})
            :FuncTy(params, ret, cfg) {}
};

// Test CheckTypeCompatibility function
TEST(UtilsTest, CheckTypeCompatibility_Nullptr)
{
    // Test null pointer case
    EXPECT_EQ(CheckTypeCompatibility(nullptr, nullptr), ark::TypeCompatibility::INCOMPATIBLE);
    auto dummyTy = new PrimitiveTy(TypeKind::TYPE_CLASS);
    EXPECT_EQ(CheckTypeCompatibility(dummyTy, nullptr), ark::TypeCompatibility::INCOMPATIBLE);
    EXPECT_EQ(CheckTypeCompatibility(nullptr, dummyTy), ark::TypeCompatibility::INCOMPATIBLE);
}

// Test IsMatchingCompletion function
TEST(UtilsTest, IsMatchingCompletion_EmptyPrefix)
{
    // Test empty prefix case
    EXPECT_TRUE(IsMatchingCompletion("", "test", true));
    EXPECT_TRUE(IsMatchingCompletion("", "test", false));
}

TEST(UtilsTest, IsMatchingCompletion_CaseSensitive)
{
    // Test case sensitive matching
    EXPECT_TRUE(IsMatchingCompletion("Te", "Test", true));
    EXPECT_FALSE(IsMatchingCompletion("te", "Test", true));
}

TEST(UtilsTest, IsMatchingCompletion_CaseInsensitive)
{
    // Test case insensitive matching
    EXPECT_TRUE(IsMatchingCompletion("te", "Test", false));
    EXPECT_TRUE(IsMatchingCompletion("TE", "Test", false));
}

// Test GetSortText function
TEST(UtilsTest, GetSortText_BoundaryValues)
{
    // Test boundary values
    EXPECT_EQ(GetSortText(0.0), "1000000");
    EXPECT_EQ(GetSortText(1.0), "0000000");
    EXPECT_EQ(GetSortText(-1.0), "1000000");
    EXPECT_EQ(GetSortText(2.0), "0000000");
}

// Test GetFilterText function
TEST(UtilsTest, GetFilterText_TestMode)
{
    // Test behavior in test mode
    EXPECT_EQ(GetFilterText("name", "prefix"), "name");
}

// Test GetNamedFuncArgRange function
TEST(UtilsTest, GetNamedFuncArgRange_NullSymbol)
{
    Node node;
    // Test case when symbol is null
    ark::Range range = GetNamedFuncArgRange(node);
    EXPECT_EQ(range.start.line, 0);
    EXPECT_EQ(range.start.column, 0);
    EXPECT_EQ(range.end.line, 0);
    EXPECT_EQ(range.end.column, 0);
}

// Test GetDeclRange function
TEST(UtilsTest, GetDeclRange_ExtendDecl)
{
    ExtendDecl decl;
    // Test ExtendDecl case (extendedType is null)
    ark::Range range = GetDeclRange(decl, 10);
    EXPECT_EQ(range.start.line, 0);
    EXPECT_EQ(range.start.column, 0);
    EXPECT_EQ(range.end.line, 0);
    EXPECT_EQ(range.end.column, 0);
}

TEST(UtilsTest, GetDeclRange_GenericParamDecl)
{
    GenericParamDecl decl;
    decl.begin = {0, 1, 1};
    decl.end = {0, 1, 5};
    // Test GenericParamDecl case
    ark::Range range = GetDeclRange(decl, 10);
    EXPECT_EQ(range.start.line, 1);
    EXPECT_EQ(range.start.column, 1);
    EXPECT_EQ(range.end.line, 1);
    EXPECT_EQ(range.end.column, 5);
}

// Test GetIdentifierRange function
TEST(UtilsTest, GetIdentifierRange_NullNode)
{
    // Test null node case
    ark::Range range = GetIdentifierRange(nullptr);
    EXPECT_EQ(range.start.line, 0);
    EXPECT_EQ(range.start.column, 0);
    EXPECT_EQ(range.end.line, 0);
    EXPECT_EQ(range.end.column, 0);
}

TEST(UtilsTest, GetIdentifierRange_NullSymbol)
{
    Node node;
    // Test case when node has no symbol
    ark::Range range = GetIdentifierRange(Ptr<Node>(&node));
    EXPECT_EQ(range.start.line, 0);
    EXPECT_EQ(range.start.column, 0);
    EXPECT_EQ(range.end.line, 0);
    EXPECT_EQ(range.end.column, 0);
}

// Test GetRefTypeRange function
TEST(UtilsTest, GetRefTypeRange_NullNode)
{
    // Test null node case
    ark::Range range = GetRefTypeRange(nullptr);
    EXPECT_EQ(range.start.line, 0);
    EXPECT_EQ(range.start.column, 0);
    EXPECT_EQ(range.end.line, 0);
    EXPECT_EQ(range.end.column, 0);
}

// Test GetCommentKind function
TEST(UtilsTest, GetCommentKind_ShortComment)
{
    // Test short comment
    EXPECT_EQ(GetCommentKind(""), ark::CommentKind::NO_COMMENT);
    EXPECT_EQ(GetCommentKind("/"), ark::CommentKind::NO_COMMENT);
}

TEST(UtilsTest, GetCommentKind_LineComment)
{
    // Test line comment
    EXPECT_EQ(GetCommentKind("// This is a comment"), ark::CommentKind::LINE_COMMENT);
}

TEST(UtilsTest, GetCommentKind_DocComment)
{
    // Test document comment
    EXPECT_EQ(GetCommentKind("/** This is a doc comment */"), ark::CommentKind::DOC_COMMENT);
}

TEST(UtilsTest, GetCommentKind_BlockComment)
{
    // Test block comment
    EXPECT_EQ(GetCommentKind("/* This is a block comment */"), ark::CommentKind::BLOCK_COMMENT);
}

TEST(UtilsTest, GetCommentKind_NoComment)
{
    // Test non-comment
    EXPECT_EQ(GetCommentKind("This is not a comment"), ark::CommentKind::NO_COMMENT);
}

// Test PrintTypeArgs function
TEST(UtilsTest, PrintTypeArgs_Empty)
{
    // Test empty type arguments
    std::vector<Ptr<Ty>> tyArgs;
    std::pair<bool, int> isVarray = {false, 0};
    EXPECT_EQ(PrintTypeArgs(tyArgs, isVarray), "");
}

// Test GetString function
TEST(UtilsTest, GetString_EmptyName)
{
    auto ty = new PrimitiveTy(TypeKind::TYPE_CSTRING);
    ty->name = "";
    // Test empty name case
    EXPECT_EQ(GetString(*ty), ty->String());
}

// Test ReplaceTuple function
TEST(UtilsTest, ReplaceTuple_NoTuple)
{
    // Test case without tuple
    EXPECT_EQ(ReplaceTuple("Int32"), "Int32");
}

TEST(UtilsTest, ReplaceTuple_WithTuple)
{
    // Test case with tuple
    EXPECT_EQ(ReplaceTuple("Tuple<Int32, String>"), "(Int32, String)");
}

// Test IsZeroPosition function
TEST(UtilsTest, IsZeroPosition_NullNode)
{
    // Test null node case
    EXPECT_FALSE(IsZeroPosition(nullptr));
}

TEST(UtilsTest, IsZeroPosition_ZeroPosition)
{
    Node node;
    node.end = {0, 0, 0};
    // Test zero position case
    EXPECT_TRUE(IsZeroPosition(Ptr<Node>(&node)));
}

TEST(UtilsTest, IsZeroPosition_NonZeroPosition)
{
    Node node;
    node.end = {0, 1, 1};
    // Test non-zero position case
    EXPECT_FALSE(IsZeroPosition(Ptr<Node>(&node)));
}

// Test ValidExtendIncludeGenericParam function
TEST(UtilsTest, ValidExtendIncludeGenericParam_NullDecl)
{
    // Test null declaration case
    EXPECT_FALSE(ValidExtendIncludeGenericParam(nullptr));
}

TEST(UtilsTest, ValidExtendIncludeGenericParam_InvalidKind)
{
    auto decl = new FuncDecl();
    // Test invalid kind case
    EXPECT_FALSE(ValidExtendIncludeGenericParam(Ptr<FuncDecl>(decl)));
}

TEST(UtilsTest, ValidExtendIncludeGenericParam_ValidKind)
{
    ClassDecl decl;
    // Test valid kind case
    EXPECT_TRUE(ValidExtendIncludeGenericParam(Ptr<Decl>(&decl)));
}

// Test SetRangForInterpolatedString function
TEST(UtilsTest, SetRangForInterpolatedString_InvalidToken)
{
    Node node;
    ark::Range range;
    Cangjie::Token token(Cangjie::TokenKind::IDENTIFIER);
    // Test invalid token type
    SetRangForInterpolatedString(token, Ptr<Node>(&node), range);
    EXPECT_EQ(range.start.line, 0);
    EXPECT_EQ(range.start.column, 0);
    EXPECT_EQ(range.end.line, 0);
    EXPECT_EQ(range.end.column, 0);
}

// Test IsFuncSignatureIdentical function
TEST(UtilsTest, IsFuncSignatureIdentical_DifferentName)
{
    FuncDecl funcDecl1, funcDecl2;
    funcDecl1.identifier = "func1";
    funcDecl2.identifier = "func2";
    // Test different name case
    EXPECT_FALSE(IsFuncSignatureIdentical(funcDecl1, funcDecl2));
}

// Test SearchContext function
TEST(UtilsTest, SearchContext_NullContext)
{
    // Test null context case
    std::vector<Symbol *> result = SearchContext(nullptr, "query");
    EXPECT_TRUE(result.empty());
}

// Test GetSymbolKind function
TEST(UtilsTest, GetSymbolKind_UnknownKind)
{
    // Test unknown kind
    EXPECT_EQ(GetSymbolKind(ASTKind::NODE), SymbolKind::NULL_KIND);
}

TEST(UtilsTest, GetSymbolKind_KnownKind)
{
    // Test known kinds
    EXPECT_EQ(GetSymbolKind(ASTKind::INTERFACE_DECL), SymbolKind::INTERFACE_DECL);
    EXPECT_EQ(GetSymbolKind(ASTKind::CLASS_DECL), SymbolKind::CLASS);
    EXPECT_EQ(GetSymbolKind(ASTKind::STRUCT_DECL), SymbolKind::STRUCT);
    EXPECT_EQ(GetSymbolKind(ASTKind::EXTEND_DECL), SymbolKind::OBJECT);
    EXPECT_EQ(GetSymbolKind(ASTKind::TYPE_ALIAS_DECL), SymbolKind::OBJECT);
    EXPECT_EQ(GetSymbolKind(ASTKind::ENUM_DECL), SymbolKind::ENUM);
    EXPECT_EQ(GetSymbolKind(ASTKind::VAR_DECL), SymbolKind::VARIABLE);
    EXPECT_EQ(GetSymbolKind(ASTKind::FUNC_DECL), SymbolKind::FUNCTION);
    EXPECT_EQ(GetSymbolKind(ASTKind::PRIMARY_CTOR_DECL), SymbolKind::FUNCTION);
    EXPECT_EQ(GetSymbolKind(ASTKind::MACRO_DECL), SymbolKind::FUNCTION);
    EXPECT_EQ(GetSymbolKind(ASTKind::MAIN_DECL), SymbolKind::FUNCTION);
    EXPECT_EQ(GetSymbolKind(ASTKind::PROP_DECL), SymbolKind::PROPERTY);
}

// Test InValidDecl function
TEST(UtilsTest, InValidDecl_NullDecl)
{
    // Test null declaration
    EXPECT_FALSE(InValidDecl(nullptr));
}

TEST(UtilsTest, InValidDecl_PrimaryCtorDecl)
{
    PrimaryCtorDecl decl;
    // Test primary constructor declaration
    EXPECT_TRUE(InValidDecl(Ptr<Decl>(&decl)));
}

// Test IsRelativePathByImported function
TEST(UtilsTest, IsRelativePathByImported_Valid)
{
    // Test relative path case
    EXPECT_TRUE(IsRelativePathByImported("module/package/file.cj"));
}

// Test IsFullPackageName function
TEST(UtilsTest, IsFullPackageName_Valid)
{
    // Test full package name case
    EXPECT_TRUE(IsFullPackageName("module.package"));
}

// Test SplitFullPackage function
TEST(UtilsTest, SplitFullPackage_WithDot)
{
    // Test package name with dot
    auto result = SplitFullPackage("module.package");
    EXPECT_EQ(result.first, "module");
    EXPECT_EQ(result.second, "package");
}

TEST(UtilsTest, SplitFullPackage_WithoutDot)
{
    // Test package name without dot
    auto result = SplitFullPackage("module");
    EXPECT_EQ(result.first, "module");
    EXPECT_EQ(result.second, "");
}

// Test PathWindowsToLinux function
TEST(UtilsTest, PathWindowsToLinux_Conversion)
{
    // Test path conversion
    EXPECT_EQ(PathWindowsToLinux("path\\to\\file"), "path/to/file");
}

// Test GetRelativePath function
TEST(UtilsTest, GetRelativePath_IdenticalPaths)
{
    // Test identical paths
    auto result = ark::GetRelativePath("/path/to/dir", "/path/to/dir");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "");
}

// Test IsMarkPos function
TEST(UtilsTest, IsMarkPos_NullNode)
{
    // Test null node
    EXPECT_FALSE(IsMarkPos(nullptr, Position{0, 1, 1}));
}

// Test LSPJoinPath function
TEST(UtilsTest, LSPJoinPath_Basic)
{
    // Test path joining
    EXPECT_EQ(LSPJoinPath("base", "append"), "base/append");
}

// Test Digest function
TEST(UtilsTest, Digest_NonExistentFile)
{
    // Test non-existent file
    EXPECT_EQ(Digest("nonexistent_file"), "");
}

// Test GetSymbolId function
TEST(UtilsTest, GetSymbolId_EmptyExportId)
{
    Decl decl;
    decl.exportId = "";
    // Test empty export ID
    EXPECT_EQ(GetSymbolId(decl), lsp::INVALID_SYMBOL_ID);
}

// Test GetFileIdForDB function
TEST(UtilsTest, GetFileIdForDB_Valid)
{
    // Test file ID generation
    EXPECT_NE(GetFileIdForDB("test_file"), 0u);
}

// Test GetConstructorIdentifier function
TEST(UtilsTest, GetConstructorIdentifier_NullFuncBody)
{
    FuncDecl decl;
    decl.funcBody = nullptr;
    // Test null function body
    EXPECT_EQ(GetConstructorIdentifier(decl, false), "");
}

// Test GetVarDeclType function
TEST(UtilsTest, GetVarDeclType_NullDecl)
{
    // Test null declaration
    EXPECT_EQ(GetVarDeclType(nullptr), "");
}

// Test GetStandardDeclAbsolutePath function
TEST(UtilsTest, GetStandardDeclAbsolutePath_BuiltinDecl)
{
    BuiltInDecl decl(BuiltInType::ARRAY);
    std::string path = "test_path";
    // Test built-in declaration
    EXPECT_EQ(GetStandardDeclAbsolutePath(Ptr<Decl>(&decl), path), "");
}

// Test IsModifierBeforeDecl function
TEST(UtilsTest, IsModifierBeforeDecl_NullDecl)
{
    // Test null declaration
    EXPECT_TRUE(IsModifierBeforeDecl(nullptr, Position{0, 1, 1}));
}

// Test Trim function
TEST(UtilsTest, Trim_EmptyString)
{
    // Test empty string
    EXPECT_EQ(Trim(""), "");
}

TEST(UtilsTest, Trim_WhitespaceString)
{
    // Test whitespace-only string
    EXPECT_EQ(Trim("   "), "");
}

TEST(UtilsTest, Trim_NormalString)
{
    // Test normal string
    EXPECT_EQ(Trim("  test  "), "test");
}

// Test GetRealPkgNameFromPath function
TEST(UtilsTest, GetRealPkgNameFromPath_Normal)
{
    // Test normal path
    EXPECT_EQ(GetRealPkgNameFromPath("test/path"), "test/path");
}

// Test CheckIsRawIdentifier function
TEST(UtilsTest, CheckIsRawIdentifier_NullNode)
{
    // Test null node
    EXPECT_FALSE(CheckIsRawIdentifier(nullptr));
}

// Test InImportSpec function
TEST(UtilsTest, InImportSpec_InvalidPosition)
{
    File file;
    // Test invalid position
    EXPECT_FALSE(InImportSpec(file, INVALID_POSITION));
}

// Test IsInCjlibDir function
TEST(UtilsTest, IsInCjlibDir_EmptyPath)
{
    // Test empty path
    EXPECT_FALSE(IsInCjlibDir(""));
}

// Test EndsWith function
TEST(UtilsTest, EndsWith_EmptyString)
{
    // Test empty string
    EXPECT_FALSE(EndsWith("", "suffix"));
}

TEST(UtilsTest, EndsWith_Valid)
{
    // Test valid suffix
    EXPECT_TRUE(EndsWith("test.txt", ".txt"));
}

TEST(UtilsTest, EndsWith_Invalid)
{
    // Test invalid suffix
    EXPECT_FALSE(EndsWith("test.txt", ".jpg"));
}

// Test RemoveFilePathExtension function
TEST(UtilsTest, RemoveFilePathExtension_NoMatch)
{
    std::string res;
    // Test non-matching extension
    EXPECT_FALSE(RemoveFilePathExtension("test.txt", ".jpg", res));
    EXPECT_EQ(res, "test.txt");
}

TEST(UtilsTest, RemoveFilePathExtension_Match)
{
    std::string res;
    // Test matching extension
    EXPECT_TRUE(RemoveFilePathExtension("test.txt", ".txt", res));
    EXPECT_EQ(res, "test");
}

// Test IsUnderPath function
TEST(UtilsTest, IsUnderPath_NotUnder)
{
    // Test not under path
    EXPECT_FALSE(IsUnderPath("/path1", "/path2"));
}

TEST(UtilsTest, IsUnderPath_Under)
{
    // Test under path
    EXPECT_TRUE(IsUnderPath("/path", "/path/subpath"));
}

// Test GetSubStrBetweenSingleQuote function
TEST(UtilsTest, GetSubStrBetweenSingleQuote_NoQuotes)
{
    // Test without quotes
    EXPECT_EQ(GetSubStrBetweenSingleQuote("test"), "");
}

TEST(UtilsTest, GetSubStrBetweenSingleQuote_Valid)
{
    // Test with valid quotes
    EXPECT_EQ(GetSubStrBetweenSingleQuote("'test'"), "test");
}

// Test GetDeclSymbolID function
TEST(UtilsTest, GetDeclSymbolID_EmptyExportId)
{
    Decl decl;
    decl.exportId = "";
    // Test empty export ID
    EXPECT_EQ(GetDeclSymbolID(decl), lsp::INVALID_SYMBOL_ID);
}

// Test IsValidIdentifier function
TEST(UtilsTest, IsValidIdentifier_Empty)
{
    // Test empty identifier
    EXPECT_FALSE(IsValidIdentifier(""));
}

TEST(UtilsTest, IsValidIdentifier_InvalidFirstChar)
{
    // Test invalid first character
    EXPECT_FALSE(IsValidIdentifier("1test"));
}

TEST(UtilsTest, IsValidIdentifier_Valid)
{
    // Test valid identifiers
    EXPECT_TRUE(IsValidIdentifier("test"));
    EXPECT_TRUE(IsValidIdentifier("_test"));
    EXPECT_TRUE(IsValidIdentifier("test123"));
}

// Test DeleteCharForPosition function
TEST(UtilsTest, DeleteCharForPosition_InvalidPosition)
{
    std::string text = "test";
    // Test invalid position
    EXPECT_FALSE(DeleteCharForPosition(text, 0, 0));
    EXPECT_FALSE(DeleteCharForPosition(text, -1, -1));
}

TEST(UtilsTest, DeleteCharForPosition_Valid)
{
    std::string text = "test";
    // Test valid position
    EXPECT_TRUE(DeleteCharForPosition(text, 1, 2));
    EXPECT_EQ(text, "tst");
}

// Test GenTaskId function
TEST(UtilsTest, GenTaskId_Valid)
{
    // Test task ID generation
    EXPECT_NE(GenTaskId("test"), 0u);
}

// Test GetSeparator function
TEST(UtilsTest, GetSeparator_Valid)
{
    // Test path separator
    char separator = GetSeparator();
    EXPECT_TRUE(separator == '/' || separator == '\\');
}

// Test IsFirstSubDir function
TEST(UtilsTest, IsFirstSubDir_Valid)
{
    // Test subdirectory check
    EXPECT_TRUE(IsFirstSubDir("/path", "/path/subdir"));
    EXPECT_FALSE(IsFirstSubDir("/path", "/otherpath"));
}

// Test GetCurTokenInTargetTokens function
TEST(UtilsTest, GetCurTokenInTargetTokens_InvalidRange)
{
    std::vector<Cangjie::Token> testTokens;

    Cangjie::Token token(Cangjie::TokenKind::IDENTIFIER);
    token.SetValue("example");
    testTokens.push_back(token);

    // 创建const引用
    const std::vector<Cangjie::Token> &tokens = testTokens;
    // Test invalid range
    EXPECT_EQ(GetCurTokenInTargetTokens(Position{0, 1, 1}, tokens, 3, 2), -1);
}

// Test remove_quotes function
TEST(UtilsTest, remove_quotes_Valid)
{
    // Test quote removal
    EXPECT_EQ(remove_quotes("\"test\""), "test");
    EXPECT_EQ(remove_quotes("'test'"), "test");
    EXPECT_EQ(remove_quotes("\"'test'\""), "test");
}

// Test GetArrayFromID function
TEST(UtilsTest, GetArrayFromID_Valid)
{
    // Test ID array generation
    IDArray result = GetArrayFromID(0x12345678);
    EXPECT_EQ(result.size(), 8u);
}

// Test GetSysCapFromDecl function
TEST(UtilsTest, GetSysCapFromDecl_NoAPILevel)
{
    Decl decl;
    auto annotation = MakeOwned<Annotation>();
    annotation->identifier = "OtherAnnotation";
    decl.annotations.push_back(std::move(annotation));
    // Test without APILevel annotation
    EXPECT_EQ(GetSysCapFromDecl(decl), "");
}

// Test FindPreFirstValidTokenKind function
TEST(UtilsTest, FindPreFirstValidTokenKind_OutOfRange)
{
    std::pair<std::string, std::string> paths = {"file_path.cj", "let x = 10;"};
    Ptr<const File> node = nullptr; // Or point to a valid File object
    DiagnosticEngine diagEngine;    // Need a DiagnosticEngine instance
    PackageInstance* pkgInstance = nullptr; // Or point to a valid PackageInstance object
    SourceManager* sm = nullptr;    // Or point to a valid SourceManager object

    // 2. Construct ArkAST variable
    ArkAST ast(paths, node, diagEngine, pkgInstance, sm);
    // Test out of range index
    EXPECT_EQ(FindPreFirstValidTokenKind(ast, 100), TokenKind::INIT);
}

// Test FindLastImportPos function
TEST(UtilsTest, FindLastImportPos_EmptyImports)
{
    File file;
    file.package = MakeOwned<PackageSpec>();
    file.package->packagePos = {0, 1, 1};
    // Test empty import list
    Position pos = FindLastImportPos(file);
    EXPECT_EQ(pos.line, 2);
    EXPECT_EQ(pos.column, 1);
}

// Test Split function
TEST(UtilsTest, Split_EmptyString)
{
    // Test empty string splitting
    auto result = Split("", ",");
    EXPECT_TRUE(result.empty());
}

TEST(UtilsTest, Split_Valid)
{
    // Test valid string splitting
    auto result = Split("a,b,c", ",");
    EXPECT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
    EXPECT_EQ(result[2], "c");
}

// Test constants definition
TEST(UtilsTest, Constants)
{
    // Test constant values
    EXPECT_EQ(NUMBER_FOR_LINE_COMMENT, 2);
    EXPECT_EQ(NUMBER_FOR_DOC_COMMENT, 3);

    // Test mapping table size
    EXPECT_EQ(AST_KIND_TO_SYMBOL_KIND.size(), 11u);
}

