// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include "SyscapCheck.cpp"
#include <unordered_set>
#include <string>
#include <vector>

using namespace ark;
using namespace Cangjie::AST;

// Test SyscapCheck constructor
TEST(SyscapCheckTest, ConstructorWithExistingModule) {
    // Prepare test data
    std::string moduleName = "testModule";
    SysCapSet testSet = {"syscap1", "syscap2"};
    SyscapCheck::module2SyscapsMap[moduleName] = testSet;

    // Create SyscapCheck instance
    SyscapCheck syscapCheck(moduleName);

    // Verify that intersectionSet is set correctly
    EXPECT_TRUE(syscapCheck.CheckSysCap("syscap1"));
    EXPECT_TRUE(syscapCheck.CheckSysCap("syscap2"));
    EXPECT_FALSE(syscapCheck.CheckSysCap("nonexistent"));

    // Clean up test data
    SyscapCheck::module2SyscapsMap.clear();
}

TEST(SyscapCheckTest, ConstructorWithNonExistingModule) {
    // Ensure module does not exist
    std::string moduleName = "nonexistentModule";
    SyscapCheck::module2SyscapsMap.clear();

    // Create SyscapCheck instance
    SyscapCheck syscapCheck(moduleName);

    // Verify that intersectionSet should be empty
    EXPECT_FALSE(syscapCheck.CheckSysCap("anySyscap"));
}

// Test SetIntersectionSet method
TEST(SyscapCheckTest, SetIntersectionSetWithExistingModule) {
    // Prepare test data
    std::string moduleName = "testModule";
    SysCapSet testSet = {"syscapA", "syscapB"};
    SyscapCheck::module2SyscapsMap[moduleName] = testSet;

    // Create SyscapCheck instance and set intersectionSet
    SyscapCheck syscapCheck("otherModule");
    syscapCheck.SetIntersectionSet(moduleName);

    // Verify that intersectionSet is set correctly
    EXPECT_TRUE(syscapCheck.CheckSysCap("syscapA"));
    EXPECT_TRUE(syscapCheck.CheckSysCap("syscapB"));
    EXPECT_FALSE(syscapCheck.CheckSysCap("nonexistent"));

    // Clean up test data
    SyscapCheck::module2SyscapsMap.clear();
}

TEST(SyscapCheckTest, SetIntersectionSetWithNonExistingModule) {
    // Ensure module does not exist
    std::string moduleName = "nonexistentModule";
    SyscapCheck::module2SyscapsMap.clear();

    // Create SyscapCheck instance and attempt to set intersectionSet
    SyscapCheck syscapCheck("testModule");
    syscapCheck.SetIntersectionSet(moduleName);

    // Verify that intersectionSet should still be empty
    EXPECT_FALSE(syscapCheck.CheckSysCap("anySyscap"));
}

// Test CheckSysCap method - Node version
TEST(SyscapCheckTest, CheckSysCapWithNode_NullNode) {
    SyscapCheck syscapCheck("");
    Ptr<Node> node = nullptr;

    // Test that null node should return true
    EXPECT_TRUE(syscapCheck.CheckSysCap(node));
}

TEST(SyscapCheckTest, CheckSysCapWithNode_NonDeclNode) {
    SyscapCheck syscapCheck("");
    auto node = Ptr<Node>(new Node());

    // Test that non-Decl node should return true
    EXPECT_TRUE(syscapCheck.CheckSysCap(node));
}

// Test CheckSysCap method - Decl version
TEST(SyscapCheckTest, CheckSysCapWithDecl_NullDecl) {
    SyscapCheck syscapCheck("");
    Ptr<Cangjie::AST::Node> decl = nullptr;

    // Test that null declaration should return true
    EXPECT_TRUE(syscapCheck.CheckSysCap(decl));
}

TEST(SyscapCheckTest, CheckSysCapWithDecl_NoAnnotations) {
    SyscapCheck syscapCheck("");
    auto decl = Ptr<Cangjie::AST::Node>(new Decl());

    // Test that declaration without annotations should return true
    EXPECT_TRUE(syscapCheck.CheckSysCap(decl));
}

TEST(SyscapCheckTest, CheckSysCapWithDecl_NonAPILevelAnnotation) {
    SyscapCheck syscapCheck("");
    auto decl = Ptr<FuncDecl>(new FuncDecl());

    // Add non-APILevel annotation
    auto annotation = Ptr<Annotation>(new Annotation());
    annotation->identifier = "OtherAnnotation";
    decl->annotations.emplace_back(annotation);

    // Test that non-APILevel annotation should return true
    EXPECT_TRUE(syscapCheck.CheckSysCap(decl));
}

TEST(SyscapCheckTest, CheckSysCapWithDecl_APILevelNoSyscapArg) {
    SyscapCheck syscapCheck("");
    auto decl = Ptr<FuncDecl>(new FuncDecl());

    // Add APILevel annotation but without syscap argument
    auto annotation = Ptr<Annotation>(new Annotation());
    annotation->identifier = "APILevel";
    decl->annotations.emplace_back(annotation);

    // Test that APILevel annotation without syscap argument should return true
    EXPECT_TRUE(syscapCheck.CheckSysCap(decl));
}

// Test CheckSysCap method - String version
TEST(SyscapCheckTest, CheckSysCapWithString_ExistingSyscap) {
    // Prepare test data
    std::string moduleName = "testModule";
    SysCapSet testSet = {"TestSysCap"};
    SyscapCheck::module2SyscapsMap[moduleName] = testSet;

    SyscapCheck syscapCheck(moduleName);

    // Test that existing syscap should return true
    EXPECT_TRUE(syscapCheck.CheckSysCap("TestSysCap"));

    // Clean up test data
    SyscapCheck::module2SyscapsMap.clear();
}

TEST(SyscapCheckTest, CheckSysCapWithString_NonExistingSyscap) {
    // Prepare test data
    std::string moduleName = "testModule";
    SysCapSet testSet = {"TestSysCap"};
    SyscapCheck::module2SyscapsMap[moduleName] = testSet;

    SyscapCheck syscapCheck(moduleName);

    // Test that non-existing syscap should return false
    EXPECT_FALSE(syscapCheck.CheckSysCap("NonExistentSysCap"));

    // Clean up test data
    SyscapCheck::module2SyscapsMap.clear();
}

TEST(SyscapCheckTest, CheckSysCapWithString_EmptyIntersectionSet) {
    SyscapCheck syscapCheck("");

    // Test that empty intersectionSet should return false
    EXPECT_FALSE(syscapCheck.CheckSysCap("AnySysCap"));
}

// Test ParseCondition method
TEST(SyscapCheckTest, ParseCondition_EmptyMap) {
    SyscapCheck syscapCheck("");
    std::unordered_map<std::string, std::string> emptyMap;

    // Test that empty map should not throw exception
    EXPECT_NO_THROW(syscapCheck.ParseCondition(emptyMap));
}

// Test helper functions
namespace {
    // Create test JSON data
    std::vector<uint8_t> CreateTestJson() {
        std::string jsonStr = R"({
            "Modules": {
                "testModule": {
                    "deviceSysCap": {
                        "paths": []
                    }
                }
            }
        })";

        return std::vector<uint8_t>(jsonStr.begin(), jsonStr.end());
    }
}

TEST(SyscapCheckTest, ParseJsonFile_ValidJson) {
    SyscapCheck syscapCheck("");
    auto jsonContent = CreateTestJson();

    // Test that parsing valid JSON should not throw exception
    EXPECT_NO_THROW(syscapCheck.ParseJsonFile(jsonContent));
}

TEST(SyscapCheckTest, ParseJsonFile_InvalidJson) {
    SyscapCheck syscapCheck("");
    std::vector<uint8_t> invalidJson = {'i', 'n', 'v', 'a', 'l', 'i', 'd'};

    // Test that parsing invalid JSON should not throw exception
    EXPECT_ANY_THROW(syscapCheck.ParseJsonFile(invalidJson));
}

// Test JSON parsing helper functions
TEST(SyscapCheckTest, ParseJsonString_Valid) {
    std::string jsonStr = "\"testString\"";
    std::vector<uint8_t> jsonBytes(jsonStr.begin(), jsonStr.end());
    size_t pos = 0;

    std::string result = ParseJsonString(pos, jsonBytes);
    EXPECT_EQ(result, "testString");
    EXPECT_EQ(pos, 11u); // Position after the closing quote
}

TEST(SyscapCheckTest, ParseJsonString_Invalid) {
    std::vector<uint8_t> jsonBytes = {'t', 'e', 's', 't'}; // No quotes
    size_t pos = 0;

    std::string result = ParseJsonString(pos, jsonBytes);
    EXPECT_EQ(result, "");
    EXPECT_EQ(pos, 0u);
}

TEST(SyscapCheckTest, ParseJsonNumber_Valid) {
    std::string jsonStr = "12345";
    std::vector<uint8_t> jsonBytes(jsonStr.begin(), jsonStr.end());
    size_t pos = 0;

    uint64_t result = ParseJsonNumber(pos, jsonBytes);
    EXPECT_EQ(result, 12345u);
}

TEST(SyscapCheckTest, ParseJsonNumber_Invalid) {
    std::vector<uint8_t> jsonBytes = {'a', 'b', 'c'};
    size_t pos = 0;

    uint64_t result = ParseJsonNumber(pos, jsonBytes);
    EXPECT_EQ(result, 0u);
}

// Test GetJsonString function
TEST(SyscapCheckTest, GetJsonString_ExistingKey) {
    auto root = MakeOwned<JsonObject>();
    auto pair = MakeOwned<JsonPair>();
    pair->key = "testKey";
    pair->valueStr = {"value1", "value2"};
    root->pairs.emplace_back(std::move(pair));

    auto result = GetJsonString(root.get(), "testKey");
    EXPECT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], "value1");
    EXPECT_EQ(result[1], "value2");
}

TEST(SyscapCheckTest, GetJsonString_NonExistingKey) {
    auto root = MakeOwned<JsonObject>();
    auto pair = MakeOwned<JsonPair>();
    pair->key = "testKey";
    pair->valueStr = {"value1", "value2"};
    root->pairs.emplace_back(std::move(pair));

    auto result = GetJsonString(root.get(), "nonExistingKey");
    EXPECT_TRUE(result.empty());
}

// Test GetJsonObject function
TEST(SyscapCheckTest, GetJsonObject_ExistingKey) {
    auto root = MakeOwned<JsonObject>();
    auto pair = MakeOwned<JsonPair>();
    pair->key = "testKey";
    pair->valueObj.emplace_back(MakeOwned<JsonObject>());
    root->pairs.emplace_back(std::move(pair));

    auto result = GetJsonObject(root.get(), "testKey", 0);
    EXPECT_NE(result, nullptr);
}

TEST(SyscapCheckTest, GetJsonObject_NonExistingKey) {
    auto root = MakeOwned<JsonObject>();
    auto pair = MakeOwned<JsonPair>();
    pair->key = "testKey";
    pair->valueObj.emplace_back(MakeOwned<JsonObject>());
    root->pairs.emplace_back(std::move(pair));

    auto result = GetJsonObject(root.get(), "nonExistingKey", 0);
    EXPECT_EQ(result, nullptr);
}

TEST(SyscapCheckTest, GetJsonObject_InvalidIndex) {
    auto root = MakeOwned<JsonObject>();
    auto pair = MakeOwned<JsonPair>();
    pair->key = "testKey";
    // Do not add any valueObj
    root->pairs.emplace_back(std::move(pair));

    auto result = GetJsonObject(root.get(), "testKey", 0);
    EXPECT_EQ(result, nullptr);
}
