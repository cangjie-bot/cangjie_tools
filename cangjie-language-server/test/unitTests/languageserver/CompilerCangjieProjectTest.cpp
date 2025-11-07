#include "SyscapCheck.cpp"
#include "SyscapCheck.h"
#include <gtest/gtest.h>
#include "Utils.h"

using namespace ark;

std::vector<uint8_t> StringToVector(const std::string &str)
{
    std::vector<uint8_t> input;
    for (char c : str) {
        input.push_back(static_cast<uint8_t>(c));
    }
    return input;
}

TEST(SyscapCheckTest, ParseJsonStringTest001)
{
    std::string str = "\"value\"";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonString(pos, input);

    EXPECT_EQ(obj, "value");
}

TEST(SyscapCheckTest, ParseJsonStringTest002)
{
    std::string str = "\"value\"";
    auto input = StringToVector(str);
    size_t pos = 10;
    auto obj = ParseJsonString(pos, input);

    EXPECT_EQ(obj, "");
}

TEST(SyscapCheckTest, ParseJsonStringTest003)
{
    std::string str = "value\"";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonString(pos, input);

    EXPECT_EQ(obj, "");
}

TEST(SyscapCheckTest, ParseJsonStringTest004)
{
    std::string str = "\"value";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonString(pos, input);

    EXPECT_EQ(obj, "value");
}

TEST(SyscapCheckTest, ParseJsonNumberTest001)
{
    std::string str = "100";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 100);
}

TEST(SyscapCheckTest, ParseJsonNumberTest002)
{
    std::string str = "a00";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 0);
}

TEST(SyscapCheckTest, ParseJsonNumberTest003)
{
    std::string str = "10a";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 10);
}

TEST(SyscapCheckTest, ParseJsonNumberTest004)
{
    std::string str = "100";
    auto input = StringToVector(str);
    size_t pos = 10;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 0);
}

TEST(SyscapCheckTest, ParseJsonNumberTest005)
{
    std::string str = "*100";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonNumber(pos, input);

    EXPECT_EQ(obj, 0);
}

TEST(SyscapCheckTest, ParseJsonArrayTest001)
{
    std::string str = "]";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto value = new JsonPair();
    ParseJsonArray(pos, input, value);
}

TEST(SyscapCheckTest, ParseJsonArrayTest002)
{
    std::string str = "[]";
    auto input = StringToVector(str);
    size_t pos = 10;
    auto value = new JsonPair();
    ParseJsonArray(pos, input, value);
}

TEST(SyscapCheckTest, ParseJsonArrayTest003)
{
    std::string str = "[\"array\" ";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto value = new JsonPair();
    ParseJsonArray(pos, input, value);
}

TEST(SyscapCheckTest, ParseJsonObjectTest001)
{
    std::string str = R"({
        "key1": "value1",
        "key2": 123,
        "key3": 12a3b,
        "key4": {"subKey": "subValue"},
        "key5": {"subKey": {"subSubKey": "value"}},
        "key6": ["array", {"obj": "val"}]
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);

    EXPECT_EQ(obj->pairs.size(), 6);
}

TEST(SyscapCheckTest, ParseJsonObjectTest002)
{
    std::string str = R"({
        "key1": "value1"
    })";
    auto input = StringToVector(str);
    size_t pos = 1000;
    auto obj = ParseJsonObject(pos, input);

    EXPECT_EQ(obj, nullptr);
}

TEST(SyscapCheckTest, ParseJsonObjectTest003)
{
    std::string str = R"(
        "key1": "value1"
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);

    EXPECT_EQ(obj, nullptr);
}

TEST(SyscapCheckTest, ParseJsonObjectTest004)
{
    std::string str = R"({
        "key1": "value1"
    )";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);

    EXPECT_EQ(obj->pairs.size(), 1);
}

TEST(SyscapCheckTest, GetJsonStringTest001)
{
    std::string str = R"({
        "key1": "value1",
        "key2": 123,
        "key3": {"subKey": "subValue"},
        "key4": {"subKey": {"subSubKey": "value"}},
        "key5": ["array", {"obj": "value"}]
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonString(obj, "subKey");

    EXPECT_EQ(res.size(), 1);
}

TEST(SyscapCheckTest, GetJsonStringTest002)
{
    std::string str = R"({
        "key1": "value1",
        "key2": 123,
        "key3": {"subKey": "subValue"},
        "key4": {"subKey": {"subSubKey": "value"}},
        "key5": ["array", {"obj": "value"}]
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonString(obj, "key1");

    EXPECT_EQ(res.size(), 1);
}

TEST(SyscapCheckTest, GetJsonStringTest003)
{
    std::string str = R"({
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonString(obj, "key1");

    EXPECT_EQ(res.size(), 0);
}

TEST(SyscapCheckTest, GetJsonStringTest004)
{
    std::string str = R"({
        "key1": "value1",
        "key2": 123,
        "key3": {"subKey": "subValue"},
        "key4": {"subKey": {"subSubKey": "value"}},
        "key5": ["array", {"obj": "value"}]
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonString(obj, "key");

    EXPECT_EQ(res.size(), 0);
}

TEST(SyscapCheckTest, GetJsonObjectTest001)
{
    std::string str = R"({
        "key1": "value1",
        "key2": 123,
        "key3": {"subKey": "subValue"},
        "key4": {"subKey": {"subSubKey": "value"}},
        "key5": ["array", {"obj": "value"}]
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonObject(obj, "key1", 0);

    EXPECT_EQ(res, nullptr);
}

TEST(SyscapCheckTest, GetJsonObjectTest002)
{
    std::string str = R"({
        "key1": "value1",
        "key2": 123,
        "key3": {"subKey": "subValue"},
        "key4": {"subKey": {"subSubKey": "value"}},
        "key5": ["array", {"obj": "value"}]
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonObject(obj, "key3", 0);

    EXPECT_NE(res, nullptr);
}

TEST(SyscapCheckTest, GetJsonObjectTest003)
{
    std::string str = R"({
        "key1": "value1",
        "key2": 123,
        "key3": {"subKey": "subValue"},
        "key4": {"subKey": {"subSubKey": "value"}},
        "key5": ["array", {"obj": [{"obj1": "value1"}, {"obj2": "value2"}]}]
    })";
    auto input = StringToVector(str);
    size_t pos = 0;
    auto obj = ParseJsonObject(pos, input);
    auto res = GetJsonObject(obj, "obj", 1);

    EXPECT_NE(res, nullptr);
}

TEST(SyscapCheckTest, ParseJsonFileTest001)
{
    std::string json = R"({
        "Modules": {
            "module1": {
                "deviceSysCap": {"key1": "val1"}
            },
            "module2": {
                "deviceSysCap": {"key2": "val2"}
            }
        }
    })";
    std::vector<uint8_t> input = StringToVector(json);
    SyscapCheck syscapCheck;
    syscapCheck.ParseJsonFile(input);

    EXPECT_EQ(syscapCheck.module2SyscapsMap.size(), 2);
}

TEST(SyscapCheckTest, ParseJsonFileTest002)
{
    std::string json = R"({
        "Modules": {
            "module1": {
                "deviceSysCap": {"key1": "val1"}
            },
            "module2": {
                "deviceSysCap": {"key2": "val2"}
            }
        }
    })";
    std::vector<uint8_t> input = StringToVector(json);
    MessageHeaderEndOfLine::SetIsDeveco(true);
    SyscapCheck syscapCheck;
    syscapCheck.ParseJsonFile(input);

    EXPECT_EQ(syscapCheck.module2SyscapsMap.size(), 4);
}

TEST(SyscapCheckTest, ParseJsonFileTest003)
{
    std::string json = R"({
        "Modules": {
            "module1": "value1"
        }
    })";
    std::vector<uint8_t> input = StringToVector(json);
    SyscapCheck syscapCheck;
    syscapCheck.ParseJsonFile(input);

    EXPECT_EQ(syscapCheck.module2SyscapsMap.size(), 4);
}

TEST(SyscapCheckTest, ParseJsonFileTest004)
{
    std::string json = R"({
        "Modules": {}
    })";
    std::vector<uint8_t> input = StringToVector(json);
    SyscapCheck syscapCheck;
    syscapCheck.ParseJsonFile(input);

    EXPECT_EQ(syscapCheck.module2SyscapsMap.size(), 4);
}

TEST(SyscapCheckTest, CheckSysCapTest001)
{
    auto declNode = new Decl();
    auto annotation = new Annotation();
    annotation->identifier = "APILevel";
    auto arg = new FuncArg();
    arg->name = "syscap";
    auto expr = new ArrayExpr();
    arg->expr = OwnedPtr<Expr>(expr);
    annotation->args.emplace_back(arg);
    declNode->annotations.emplace_back(annotation);

    auto hasAPILevel = true;
    SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(declNode, hasAPILevel);

    EXPECT_TRUE(result);
    EXPECT_TRUE(hasAPILevel);
}

TEST(SyscapCheckTest, CheckSysCapTest002)
{
    auto declNode = new Decl();
    auto annotation = new Annotation();
    annotation->identifier = "APILevel";
    auto arg = new FuncArg();
    arg->name = "syscap";
    annotation->args.emplace_back(arg);
    declNode->annotations.emplace_back(annotation);

    auto hasAPILevel = true;
    SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(declNode, hasAPILevel);

    EXPECT_TRUE(result);
    EXPECT_TRUE(hasAPILevel);
}

TEST(SyscapCheckTest, CheckSysCapTest003)
{
    auto declNode = new Decl();
    auto annotation = new Annotation();
    annotation->identifier = "APILevel";
    auto arg = new FuncArg();
    arg->name = "syscap";
    auto expr = new LitConstExpr();
    expr->kind = LitConstKind::STRING;
    arg->expr = OwnedPtr<Expr>(expr);
    annotation->args.emplace_back(arg);
    declNode->annotations.emplace_back(annotation);

    auto hasAPILevel = true;
    SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(declNode, hasAPILevel);

    EXPECT_FALSE(result);
    EXPECT_TRUE(hasAPILevel);
}

TEST(SyscapCheckTest, CheckSysCapTest004)
{
    auto declNode = new Decl();
    auto annotation = new Annotation();
    annotation->identifier = "APILevel";
    auto arg = new FuncArg();
    arg->name = "syscap";
    auto expr = new ArrayExpr();
    arg->expr = OwnedPtr<Expr>(expr);
    annotation->args.emplace_back(arg);
    declNode->annotations.emplace_back(annotation);

    const SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(declNode);

    EXPECT_TRUE(result);
}

TEST(SyscapCheckTest, CheckSysCapTest005)
{
    auto declNode = new Decl();
    auto annotation = new Annotation();
    annotation->identifier = "APILevel";
    auto arg = new FuncArg();
    arg->name = "syscap";
    annotation->args.emplace_back(arg);
    declNode->annotations.emplace_back(annotation);

    const SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(declNode);

    EXPECT_TRUE(result);
}

TEST(SyscapCheckTest, CheckSysCapTest006)
{
    auto declNode = new Decl();
    auto annotation = new Annotation();
    annotation->identifier = "APILevel";
    auto arg = new FuncArg();
    arg->name = "syscap";
    auto expr = new LitConstExpr();
    expr->kind = LitConstKind::STRING;
    arg->expr = OwnedPtr<Expr>(expr);
    annotation->args.emplace_back(arg);
    declNode->annotations.emplace_back(annotation);

    const SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(declNode);

    EXPECT_FALSE(result);
}

TEST(SyscapCheckTest, CheckSysCapTest007)
{
    const SyscapCheck syscapCheck;
    auto result = syscapCheck.CheckSysCap(nullptr);

    EXPECT_TRUE(result);
}

TEST(SyscapCheckTest, CheckSysCapTest012)
{
    const std::string& syscapName = "syscap1";
    SyscapCheck syscapCheck;
    auto res = syscapCheck.CheckSysCap(syscapName);

    EXPECT_EQ(res, false);
}