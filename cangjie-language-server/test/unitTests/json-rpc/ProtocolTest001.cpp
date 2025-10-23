// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "Protocol.h"

using json = nlohmann::json;
using namespace ark;

class ProtocolTest001 : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset global state before each test
        MessageHeaderEndOfLine::SetIsDeveco(false);
    }
};

// Test case for DidOpenTextDocumentParams FromJSON
TEST_F(ProtocolTest001, FromJSON_DidOpenTextDocumentParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj",
            "languageId": "Cangjie",
            "version": 1,
            "text": "fn main() {}"
        }
    })"_json;

    DidOpenTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.textDocument.languageId, "Cangjie");
    EXPECT_EQ(reply.textDocument.version, 1);
    EXPECT_EQ(reply.textDocument.text, "fn main() {}");
}

TEST_F(ProtocolTest001, FromJSON_DidOpenTextDocumentParams_MissingFields) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj",
            "languageId": "Cangjie"
        }
    })"_json;

    DidOpenTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}


// Test case for TextDocumentPositionParams FromJSON
TEST_F(ProtocolTest001, FromJSON_TextDocumentPositionParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        }
    })"_json;

    TextDocumentPositionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
}

TEST_F(ProtocolTest001, FromJSON_TextDocumentPositionParams_InvalidStructure) {
    json params = R"({
        "textDocument": "invalid",
        "position": {
            "line": 10,
            "character": 5
        }
    })"_json;

    TextDocumentPositionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}

// Test case for CrossLanguageJumpParams FromJSON
TEST_F(ProtocolTest001, FromJSON_CrossLanguageJumpParams_ValidInput) {
    json params = R"({
        "packageName": "com.example",
        "name": "MyClass",
        "outerName": "Outer",
        "isCombined": true
    })"_json;

    CrossLanguageJumpParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.packageName, "com.example");
    EXPECT_EQ(reply.name, "MyClass");
    EXPECT_EQ(reply.outerName, "Outer");
    EXPECT_TRUE(reply.isCombined);
}

TEST_F(ProtocolTest001, FromJSON_CrossLanguageJumpParams_OptionalFieldsMissing) {
    json params = R"({
        "packageName": "com.example",
        "name": "MyClass"
    })"_json;

    CrossLanguageJumpParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.packageName, "com.example");
    EXPECT_EQ(reply.name, "MyClass");
    EXPECT_EQ(reply.outerName, "");  // Default value
    EXPECT_FALSE(reply.isCombined);  // Default value
}

// Test case for OverrideMethodsParams FromJSON
TEST_F(ProtocolTest001, FromJSON_OverrideMethodsParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        },
        "isExtend": true
    })"_json;

    OverrideMethodsParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
    EXPECT_TRUE(reply.isExtend);
}

// Test case for ExportsNameParams FromJSON
TEST_F(ProtocolTest001, FromJSON_ExportsNameParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        },
        "packageName": "com.example"
    })"_json;

    ExportsNameParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
    EXPECT_EQ(reply.packageName, "com.example");
}

TEST_F(ProtocolTest001, FromJSON_SignatureHelpContext_InvalidTriggerKind) {
    json params = R"({
        "triggerKind": -1
    })"_json;

    SignatureHelpContext reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}

// Test case for SignatureHelpParams FromJSON
TEST_F(ProtocolTest001, FromJSON_SignatureHelpParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        },
        "context": {
            "triggerKind": 1
        }
    })"_json;

    SignatureHelpParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
}

// Test case for InitializeParams FromJSON
TEST_F(ProtocolTest001, FromJSON_InitializeParams_ValidInput) {
    json params = R"({
        "rootUri": "file:///workspace",
        "capabilities": {
            "textDocument": {
                "documentHighlight": {},
                "typeHierarchy": {},
                "publishDiagnostics": {
                    "versionSupport": true
                },
                "hover": {},
                "documentLink": {}
            }
        },
        "initializationOptions": {
            "cangjieRootUri": "file:///custom_root"
        }
    })"_json;

    InitializeParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.rootUri.file, "file:///custom_root");
    EXPECT_TRUE(MessageHeaderEndOfLine::GetIsDeveco());
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.documentHighlightClientCapabilities);
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.typeHierarchyCapabilities);
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.diagnosticVersionSupport);
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.hoverClientCapabilities);
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.documentLinkClientCapabilities);
}

// Test case for DidCloseTextDocumentParams FromJSON
TEST_F(ProtocolTest001, FromJSON_DidCloseTextDocumentParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        }
    })"_json;

    DidCloseTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

// Test case for TrackCompletionParams FromJSON
TEST_F(ProtocolTest001, FromJSON_TrackCompletionParams_ValidInput) {
    json params = R"({
        "label": "myFunction"
    })"_json;

    TrackCompletionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.label, "myFunction");
}

TEST_F(ProtocolTest001, FromJSON_TrackCompletionParams_MissingLabel) {
    json params = R"({
        "otherField": "value"
    })"_json;

    TrackCompletionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_FALSE(result);
}

// Test case for CompletionContext FromJSON
TEST_F(ProtocolTest001, FromJSON_CompletionContext_ValidInput) {
    json params = R"({
        "triggerKind": 2,
        "triggerCharacter": "."
    })"_json;

    CompletionContext reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(static_cast<int>(reply.triggerKind), 2);
    EXPECT_EQ(reply.triggerCharacter, ".");
}

// Test case for CompletionParams FromJSON
TEST_F(ProtocolTest001, FromJSON_CompletionParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        },
        "context": {
            "triggerKind": 1
        }
    })"_json;

    CompletionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
    EXPECT_EQ(static_cast<int>(reply.context.triggerKind), 1);
}

// Test case for SemanticTokensParams FromJSON
TEST_F(ProtocolTest001, FromJSON_SemanticTokensParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        }
    })"_json;

    SemanticTokensParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

// Test case for DidChangeTextDocumentParams FromJSON
TEST_F(ProtocolTest001, FromJSON_DidChangeTextDocumentParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj",
            "version": 2
        },
        "contentChanges": [
            {
                "text": "updated text",
                "range": {
                    "start": {"line": 0, "character": 0},
                    "end": {"line": 0, "character": 10}
                },
                "rangeLength": 10
            }
        ]
    })"_json;

    DidChangeTextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.textDocument.version, 2);
    ASSERT_EQ(reply.contentChanges.size(), 1u);
    EXPECT_EQ(reply.contentChanges[0].text, "updated text");
    EXPECT_EQ(reply.contentChanges[0].range->start.line, 0);
    EXPECT_EQ(reply.contentChanges[0].range->start.column, 0);
    EXPECT_EQ(reply.contentChanges[0].range->end.line, 0);
    EXPECT_EQ(reply.contentChanges[0].range->end.column, 10);
    EXPECT_EQ(reply.contentChanges[0].rangeLength, 10);
}

// Test case for RenameParams FromJSON
TEST_F(ProtocolTest001, FromJSON_RenameParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "position": {
            "line": 10,
            "character": 5
        },
        "newName": "newVarName"
    })"_json;

    RenameParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
    EXPECT_EQ(reply.newName, "newVarName");
}

// Test case for TextDocumentIdentifier FromJSON
TEST_F(ProtocolTest001, FromJSON_TextDocumentIdentifier_ValidInput) {
    json params = R"({
        "uri": "file:///test.cj"
    })"_json;

    TextDocumentIdentifier reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.uri.file, "file:///test.cj");
}

// Test case for TextDocumentParams FromJSON
TEST_F(ProtocolTest001, FromJSON_TextDocumentParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        }
    })"_json;

    TextDocumentParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

// Test case for TypeHierarchyItem FromJSON
TEST_F(ProtocolTest001, FromJSON_TypeHierarchyItem_ValidInput) {
    json params = R"({
        "item": {
            "name": "MyClass",
            "kind": 5,
            "uri": "file:///test.cj",
            "range": {
                "start": {"line": 0, "character": 0},
                "end": {"line": 10, "character": 20}
            },
            "selectionRange": {
                "start": {"line": 2, "character": 5},
                "end": {"line": 2, "character": 15}
            },
            "data": {
                "isKernel": true,
                "isChildOrSuper": false,
                "symbolId": "12345"
            }
        }
    })"_json;

    TypeHierarchyItem reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.name, "MyClass");
    EXPECT_EQ(static_cast<int>(reply.kind), 5);
    EXPECT_EQ(reply.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.range.start.line, 0);
    EXPECT_EQ(reply.range.start.column, 0);
    EXPECT_EQ(reply.range.end.line, 10);
    EXPECT_EQ(reply.range.end.column, 20);
    EXPECT_EQ(reply.selectionRange.start.line, 2);
    EXPECT_EQ(reply.selectionRange.start.column, 5);
    EXPECT_EQ(reply.selectionRange.end.line, 2);
    EXPECT_EQ(reply.selectionRange.end.column, 15);
    EXPECT_TRUE(reply.isKernel);
    EXPECT_FALSE(reply.isChildOrSuper);
    EXPECT_EQ(reply.symbolId, 12345ull);
}

// Test case for CallHierarchyItem FromJSON
TEST_F(ProtocolTest001, FromJSON_CallHierarchyItem_ValidInput) {
    json params = R"({
        "item": {
            "name": "myMethod",
            "kind": 6,
            "uri": "file:///test.cj",
            "range": {
                "start": {"line": 5, "character": 10},
                "end": {"line": 7, "character": 20}
            },
            "selectionRange": {
                "start": {"line": 6, "character": 15},
                "end": {"line": 6, "character": 25}
            },
            "detail": "This is a method",
            "data": {
                "isKernel": false,
                "symbolId": "67890"
            }
        }
    })"_json;

    CallHierarchyItem reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.name, "myMethod");
    EXPECT_EQ(static_cast<int>(reply.kind), 6);
    EXPECT_EQ(reply.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.range.start.line, 5);
    EXPECT_EQ(reply.range.start.column, 10);
    EXPECT_EQ(reply.range.end.line, 7);
    EXPECT_EQ(reply.range.end.column, 20);
    EXPECT_EQ(reply.selectionRange.start.line, 6);
    EXPECT_EQ(reply.selectionRange.start.column, 15);
    EXPECT_EQ(reply.selectionRange.end.line, 6);
    EXPECT_EQ(reply.selectionRange.end.column, 25);
    EXPECT_EQ(reply.detail, "This is a method");
    EXPECT_FALSE(reply.isKernel);
    EXPECT_EQ(reply.symbolId, 67890ull);
}

// Test case for DocumentLinkParams FromJSON
TEST_F(ProtocolTest001, FromJSON_DocumentLinkParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        }
    })"_json;

    DocumentLinkParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

// Test case for DidChangeWatchedFilesParam FromJSON
TEST_F(ProtocolTest001, FromJSON_DidChangeWatchedFilesParam_ValidInput) {
    json params = R"({
        "changes": [
            {
                "uri": "file:///test.cj",
                "type": 1
            }
        ]
    })"_json;

    DidChangeWatchedFilesParam reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    ASSERT_EQ(reply.changes.size(), 1u);
    EXPECT_EQ(reply.changes[0].textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(static_cast<int>(reply.changes[0].type), 1);
}

// Test case for DiagnosticRelatedInformation FromJSON
TEST_F(ProtocolTest001, FromJSON_DiagnosticRelatedInformation_ValidInput) {
    json param = R"({
        "message": "Defined here",
        "location": {
            "uri": "file:///definition.cj",
            "range": {
                "start": {"line": 10, "character": 5},
                "end": {"line": 10, "character": 15}
            }
        }
    })"_json;

    DiagnosticRelatedInformation info;
    bool result = FromJSON(param, info);

    EXPECT_TRUE(result);
    EXPECT_EQ(info.message, "Defined here");
    EXPECT_EQ(info.location.uri.file, "file:///definition.cj");
    EXPECT_EQ(info.location.range.start.line, 10);
    EXPECT_EQ(info.location.range.start.column, 5);
    EXPECT_EQ(info.location.range.end.line, 10);
    EXPECT_EQ(info.location.range.end.column, 15);
}

// Test case for DocumentSymbolParams FromJSON
TEST_F(ProtocolTest001, FromJSON_DocumentSymbolParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        }
    })"_json;

    DocumentSymbolParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

// Test case for CodeActionParams FromJSON
TEST_F(ProtocolTest001, FromJSON_CodeActionParams_ValidInput) {
    json params = R"({
        "textDocument": {
            "uri": "file:///test.cj"
        },
        "range": {
            "start": {"line": 5, "character": 10},
            "end": {"line": 5, "character": 20}
        },
        "context": {
            "diagnostics": []
        }
    })"_json;

    CodeActionParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.range.start.line, 5);
    EXPECT_EQ(reply.range.start.column, 10);
    EXPECT_EQ(reply.range.end.line, 5);
    EXPECT_EQ(reply.range.end.column, 20);
}

// Test case for TweakArgs FromJSON
TEST_F(ProtocolTest001, FromJSON_TweakArgs_ValidInput) {
    json params = R"({
        "file": "file:///test.cj",
        "selection": {
            "start": {"line": 5, "character": 10},
            "end": {"line": 5, "character": 20}
        },
        "tweakID": "rename-variable",
        "extraOptions": {
            "newName": "newVar"
        }
    })"_json;

    TweakArgs reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.file.file, "file:///test.cj");
    EXPECT_EQ(reply.selection.start.line, 5);
    EXPECT_EQ(reply.selection.start.column, 10);
    EXPECT_EQ(reply.selection.end.line, 5);
    EXPECT_EQ(reply.selection.end.column, 20);
    EXPECT_EQ(reply.tweakID, "rename-variable");
    ASSERT_EQ(reply.extraOptions.size(), 1u);
    EXPECT_EQ(reply.extraOptions.at("newName"), "newVar");
}

// Test case for ExecuteCommandParams FromJSON
TEST_F(ProtocolTest001, FromJSON_ExecuteCommandParams_ValidInput) {
    json params = R"({
        "command": "cjLsp.applyTweak",
        "arguments": [
            {
                "file": "file:///test.cj",
                "selection": {
                    "start": {"line": 5, "character": 10},
                    "end": {"line": 5, "character": 20}
                },
                "tweakID": "rename-variable"
            }
        ]
    })"_json;

    ExecuteCommandParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.command, "cjLsp.applyTweak");
    EXPECT_FALSE(reply.arguments.empty());
}

// Test case for FileRefactorReqParams FromJSON
TEST_F(ProtocolTest001, FromJSON_FileRefactorReqParams_ValidInput) {
    json params = R"({
        "file": {
            "uri": "file:///old/path.cj"
        },
        "targetPath": {
            "uri": "file:///new/path.cj"
        },
        "selectedElement": {
            "uri": "file:///element.cj"
        }
    })"_json;

    FileRefactorReqParams reply;
    bool result = FromJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply.file.uri.file, "file:///old/path.cj");
    EXPECT_EQ(reply.targetPath.uri.file, "file:///new/path.cj");
    EXPECT_EQ(reply.selectedElement.uri.file, "file:///element.cj");
}

// Test cases for ToJSON functions would follow similar patterns...
// For brevity, only one example is shown below:

// Test case for BreakpointLocation ToJSON
TEST_F(ProtocolTest001, ToJSON_BreakpointLocation_ValidInput) {
    BreakpointLocation params;
    params.uri = "file:///test.cj";
    params.range.start.line = 5;
    params.range.start.column = 10;
    params.range.end.line = 5;
    params.range.end.column = 20;

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["uri"], "file:///test.cj");
    EXPECT_EQ(reply["range"]["start"]["line"], 5);
    EXPECT_EQ(reply["range"]["start"]["character"], 10);
    EXPECT_EQ(reply["range"]["end"]["line"], 5);
    EXPECT_EQ(reply["range"]["end"]["character"], 20);
}

// Test case for ExecutableRange ToJSON
TEST_F(ProtocolTest001, ToJSON_ExecutableRange_ValidInput) {
    ExecutableRange params;
    params.uri = "file:///test.cj";
    params.projectName = "MyProject";
    params.packageName = "com.example";
    params.className = "MyClass";
    params.functionName = "main";
    params.range.start.line = 0;
    params.range.start.column = 0;
    params.range.end.line = 10;
    params.range.end.column = 20;

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["uri"], "file:///test.cj");
    EXPECT_EQ(reply["projectName"], "MyProject");
    EXPECT_EQ(reply["packageName"], "com.example");
    EXPECT_EQ(reply["className"], "MyClass");
    EXPECT_EQ(reply["functionName"], "main");
    EXPECT_EQ(reply["range"]["start"]["line"], 0);
    EXPECT_EQ(reply["range"]["start"]["character"], 0);
    EXPECT_EQ(reply["range"]["end"]["line"], 10);
    EXPECT_EQ(reply["range"]["end"]["character"], 20);
}

// Test case for CodeLens ToJSON
TEST_F(ProtocolTest001, ToJSON_CodeLens_ValidInput) {
    CodeLens params;
    params.range.start.line = 5;
    params.range.start.column = 10;
    params.range.end.line = 5;
    params.range.end.column = 20;

    params.command.title = "Run Test";
    params.command.command = "run.test";

    ExecutableRange arg;
    arg.uri = "file:///test.cj";
    arg.range.start.line = 0;
    arg.range.start.column = 0;
    arg.range.end.line = 10;
    arg.range.end.column = 20;
    params.command.arguments.insert(arg);

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["range"]["start"]["line"], 5);
    EXPECT_EQ(reply["range"]["start"]["character"], 10);
    EXPECT_EQ(reply["range"]["end"]["line"], 5);
    EXPECT_EQ(reply["range"]["end"]["character"], 20);
    EXPECT_EQ(reply["command"]["title"], "Run Test");
    EXPECT_EQ(reply["command"]["command"], "run.test");
    ASSERT_EQ(reply["command"]["arguments"].size(), 1u);
    EXPECT_EQ(reply["command"]["arguments"][0]["uri"], "file:///test.cj");
}

// Test case for Command ToJSON
TEST_F(ProtocolTest001, ToJSON_Command_ValidInput) {
    Command params;
    params.title = "Apply Fix";
    params.command = "apply.fix";

    ExecutableRange arg;
    arg.uri = "file:///test.cj";
    arg.tweakId = "fix-imports";
    arg.range.start.line = 0;
    arg.range.start.column = 0;
    arg.range.end.line = 10;
    arg.range.end.column = 20;
    arg.projectName = "TestProject";
    arg.packageName = "com.example";
    arg.className = "TestClass";
    arg.functionName = "testFunction";
    arg.extraOptions["option1"] = "value1";
    params.arguments.insert(arg);

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["title"], "Apply Fix");
    EXPECT_EQ(reply["command"], "apply.fix");
    ASSERT_EQ(reply["arguments"].size(), 1u);
    EXPECT_EQ(reply["arguments"][0]["tweakID"], "fix-imports");
    EXPECT_EQ(reply["arguments"][0]["file"], "file:///test.cj");
    EXPECT_EQ(reply["arguments"][0]["selection"]["start"]["line"], 0);
    EXPECT_EQ(reply["arguments"][0]["selection"]["start"]["character"], 0);
    EXPECT_EQ(reply["arguments"][0]["selection"]["end"]["line"], 10);
    EXPECT_EQ(reply["arguments"][0]["selection"]["end"]["character"], 20);
    EXPECT_EQ(reply["arguments"][0]["projectName"], "TestProject");
    EXPECT_EQ(reply["arguments"][0]["packageName"], "com.example");
    EXPECT_EQ(reply["arguments"][0]["className"], "TestClass");
    EXPECT_EQ(reply["arguments"][0]["functionName"], "testFunction");
    EXPECT_EQ(reply["arguments"][0]["option1"], "value1");
}

// Test case for TypeHierarchyItem ToJSON
TEST_F(ProtocolTest001, ToJSON_TypeHierarchyItem_ValidInput) {
    TypeHierarchyItem iter;
    iter.name = "MyClass";
    iter.kind = SymbolKind::CLASS;
    iter.uri.file = "file:///test.cj";
    iter.range.start.line = 0;
    iter.range.start.column = 0;
    iter.range.end.line = 10;
    iter.range.end.column = 20;
    iter.selectionRange.start.line = 2;
    iter.selectionRange.start.column = 5;
    iter.selectionRange.end.line = 2;
    iter.selectionRange.end.column = 15;
    iter.isKernel = true;
    iter.isChildOrSuper = false;
    iter.symbolId = 12345ull;

    json replyTH;
    bool result = ToJSON(iter, replyTH);

    EXPECT_TRUE(result);
    EXPECT_EQ(replyTH["name"], "MyClass");
    EXPECT_EQ(replyTH["kind"], static_cast<int>(SymbolKind::CLASS));
    EXPECT_EQ(replyTH["uri"], "file:///test.cj");
    EXPECT_EQ(replyTH["range"]["start"]["line"], 0);
    EXPECT_EQ(replyTH["range"]["start"]["character"], 0);
    EXPECT_EQ(replyTH["range"]["end"]["line"], 10);
    EXPECT_EQ(replyTH["range"]["end"]["character"], 20);
    EXPECT_EQ(replyTH["selectionRange"]["start"]["line"], 2);
    EXPECT_EQ(replyTH["selectionRange"]["start"]["character"], 5);
    EXPECT_EQ(replyTH["selectionRange"]["end"]["line"], 2);
    EXPECT_EQ(replyTH["selectionRange"]["end"]["character"], 15);
    EXPECT_TRUE(replyTH["data"]["isKernel"]);
    EXPECT_FALSE(replyTH["data"]["isChildOrSuper"]);
    EXPECT_EQ(replyTH["data"]["symbolId"], "12345");
}

// Test case for CallHierarchyItem ToJSON
TEST_F(ProtocolTest001, ToJSON_CallHierarchyItem_ValidInput) {
    CallHierarchyItem iter;
    iter.name = "myMethod";
    iter.kind = SymbolKind::FUNCTION;
    iter.uri.file = "file:///test.cj";
    iter.range.start.line = 5;
    iter.range.start.column = 10;
    iter.range.end.line = 7;
    iter.range.end.column = 20;
    iter.selectionRange.start.line = 6;
    iter.selectionRange.start.column = 15;
    iter.selectionRange.end.line = 6;
    iter.selectionRange.end.column = 25;
    iter.detail = "This is a method";
    iter.isKernel = false;
    iter.symbolId = 67890ull;

    json replyCH;
    bool result = ToJSON(iter, replyCH);

    EXPECT_TRUE(result);
    EXPECT_EQ(replyCH["name"], "myMethod");
    EXPECT_EQ(replyCH["kind"], static_cast<int>(SymbolKind::FUNCTION));
    EXPECT_EQ(replyCH["uri"], "file:///test.cj");
    EXPECT_EQ(replyCH["range"]["start"]["line"], 5);
    EXPECT_EQ(replyCH["range"]["start"]["character"], 10);
    EXPECT_EQ(replyCH["range"]["end"]["line"], 7);
    EXPECT_EQ(replyCH["range"]["end"]["character"], 20);
    EXPECT_EQ(replyCH["selectionRange"]["start"]["line"], 6);
    EXPECT_EQ(replyCH["selectionRange"]["start"]["character"], 15);
    EXPECT_EQ(replyCH["selectionRange"]["end"]["line"], 6);
    EXPECT_EQ(replyCH["selectionRange"]["end"]["character"], 25);
    EXPECT_EQ(replyCH["detail"], "This is a method");
    EXPECT_FALSE(replyCH["data"]["isKernel"]);
    EXPECT_EQ(replyCH["data"]["symbolId"], "67890");
}

// Test case for CompletionItem ToJSON
TEST_F(ProtocolTest001, ToJSON_CompletionItem_ValidInput) {
    CompletionItem iter;
    iter.label = "myFunction";
    iter.kind = CompletionItemKind::CIK_FUNCTION;
    iter.detail = "A sample function";
    iter.documentation = "Does something useful";
    iter.sortText = "a";
    iter.filterText = "myFunc";
    iter.insertText = "myFunction()";
    iter.insertTextFormat = InsertTextFormat::SNIPPET;
    iter.deprecated = false;

    TextEdit edit;
    edit.range.start.line = 0;
    edit.range.start.column = 0;
    edit.range.end.line = 0;
    edit.range.end.column = 10;
    edit.newText = "replacement";
    iter.additionalTextEdits = std::vector{edit};

    json reply;
    bool result = ToJSON(iter, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["label"], "myFunction");
    EXPECT_EQ(reply["kind"], static_cast<int>(CompletionItemKind::CIK_FUNCTION));
    EXPECT_EQ(reply["detail"], "A sample function");
    EXPECT_EQ(reply["documentation"], "Does something useful");
    EXPECT_EQ(reply["sortText"], "a");
    EXPECT_EQ(reply["filterText"], "myFunc");
    EXPECT_EQ(reply["insertText"], "myFunction()");
    EXPECT_EQ(reply["insertTextFormat"], static_cast<int>(InsertTextFormat::SNIPPET));
    EXPECT_FALSE(reply["deprecated"]);
    ASSERT_TRUE(reply.contains("additionalTextEdits"));
    ASSERT_EQ(reply["additionalTextEdits"].size(), 1u);
    EXPECT_EQ(reply["additionalTextEdits"][0]["newText"], "replacement");
}

// Test case for DiagnosticRelatedInformation ToJSON
TEST_F(ProtocolTest001, ToJSON_DiagnosticRelatedInformation_ValidInput) {
    DiagnosticRelatedInformation info;
    info.message = "Defined here";
    info.location.uri.file = "file:///definition.cj";
    info.location.range.start.line = 10;
    info.location.range.start.column = 5;
    info.location.range.end.line = 10;
    info.location.range.end.column = 15;

    json reply;
    bool result = ToJSON(info, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["message"], "Defined here");
    EXPECT_EQ(reply["location"]["uri"], "file:///definition.cj");
    EXPECT_EQ(reply["location"]["range"]["start"]["line"], 10);
    EXPECT_EQ(reply["location"]["range"]["start"]["character"], 5);
    EXPECT_EQ(reply["location"]["range"]["end"]["line"], 10);
    EXPECT_EQ(reply["location"]["range"]["end"]["character"], 15);
}

// Test case for PublishDiagnosticsParams ToJSON
TEST_F(ProtocolTest001, ToJSON_PublishDiagnosticsParams_ValidInput) {
    PublishDiagnosticsParams params;
    params.uri.file = "file:///test.cj";
    params.version = 1;

    DiagnosticToken diag;
    diag.range.start.line = 5;
    diag.range.start.column = 10;
    diag.range.end.line = 5;
    diag.range.end.column = 20;
    diag.severity = 1;
    diag.source = "compiler";
    diag.message = "Undefined variable 'x'";
    params.diagnostics.push_back(diag);

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["uri"], "file:///test.cj");
    EXPECT_EQ(reply["version"], 1);
    ASSERT_TRUE(reply.contains("diagnostics"));
    ASSERT_EQ(reply["diagnostics"].size(), 1u);
    EXPECT_EQ(reply["diagnostics"][0]["message"], "Undefined variable 'x'");
}

// Test case for WorkspaceEdit ToJSON
TEST_F(ProtocolTest001, ToJSON_WorkspaceEdit_ValidInput) {
    WorkspaceEdit params;

    TextEdit edit;
    edit.range.start.line = 0;
    edit.range.start.column = 0;
    edit.range.end.line = 0;
    edit.range.end.column = 10;
    edit.newText = "new content";

    params.changes["file:///test.cj"] = std::vector{edit};

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    ASSERT_TRUE(reply.contains("changes"));
    ASSERT_TRUE(reply["changes"].contains("file:///test.cj"));
    ASSERT_EQ(reply["changes"]["file:///test.cj"].size(), 1u);
    EXPECT_EQ(reply["changes"]["file:///test.cj"][0]["newText"], "new content");
}

// Test case for TextDocumentEdit ToJSON
TEST_F(ProtocolTest001, ToJSON_TextDocumentEdit_ValidInput) {
    TextDocumentEdit params;
    params.textDocument.uri.file = "file:///test.cj";
    params.textDocument.version = 1;

    TextEdit edit;
    edit.range.start.line = 0;
    edit.range.start.column = 0;
    edit.range.end.line = 0;
    edit.range.end.column = 10;
    edit.newText = "new content";
    params.textEdits.push_back(edit);

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["textDocument"]["uri"], "file:///test.cj");
    EXPECT_EQ(reply["textDocument"]["version"], 1);
    ASSERT_EQ(reply["edits"].size(), 1u);
    EXPECT_EQ(reply["edits"][0]["newText"], "new content");
}

// Test case for DocumentSymbol ToJSON
TEST_F(ProtocolTest001, ToJSON_DocumentSymbol_ValidInput) {
    DocumentSymbol item;
    item.name = "MyClass";
    item.kind = SymbolKind::CLASS;
    item.detail = "A sample class";
    item.range.start.line = 0;
    item.range.start.column = 0;
    item.range.end.line = 10;
    item.range.end.column = 20;
    item.selectionRange.start.line = 2;
    item.selectionRange.start.column = 5;
    item.selectionRange.end.line = 2;
    item.selectionRange.end.column = 15;

    DocumentSymbol child;
    child.name = "myMethod";
    child.kind = SymbolKind::FUNCTION;
    child.range.start.line = 3;
    child.range.start.column = 5;
    child.range.end.line = 5;
    child.range.end.column = 15;
    child.selectionRange.start.line = 4;
    child.selectionRange.start.column = 7;
    child.selectionRange.end.line = 4;
    child.selectionRange.end.column = 13;
    item.children.push_back(child);

    json result;
    bool success = ToJSON(item, result);

    EXPECT_TRUE(success);
    EXPECT_EQ(result["name"], "MyClass");
    EXPECT_EQ(result["kind"], static_cast<int>(SymbolKind::CLASS));
    EXPECT_EQ(result["detail"], "A sample class");
    EXPECT_EQ(result["range"]["start"]["line"], 0);
    EXPECT_EQ(result["range"]["start"]["character"], 0);
    EXPECT_EQ(result["range"]["end"]["line"], 10);
    EXPECT_EQ(result["range"]["end"]["character"], 20);
    EXPECT_EQ(result["selectionRange"]["start"]["line"], 2);
    EXPECT_EQ(result["selectionRange"]["start"]["character"], 5);
    EXPECT_EQ(result["selectionRange"]["end"]["line"], 2);
    EXPECT_EQ(result["selectionRange"]["end"]["character"], 15);
    ASSERT_TRUE(result.contains("children"));
    ASSERT_EQ(result["children"].size(), 1u);
    EXPECT_EQ(result["children"][0]["name"], "myMethod");
}

// Test case for CallHierarchyOutgoingCall ToJSON
TEST_F(ProtocolTest001, ToJSON_CallHierarchyOutgoingCall_ValidInput) {
    CallHierarchyOutgoingCall iter;

    CallHierarchyItem toItem;
    toItem.name = "calledFunction";
    toItem.kind = SymbolKind::FUNCTION;
    toItem.uri.file = "file:///callee.cj";
    toItem.range.start.line = 0;
    toItem.range.start.column = 0;
    toItem.range.end.line = 5;
    toItem.range.end.column = 20;
    toItem.selectionRange.start.line = 1;
    toItem.selectionRange.start.column = 5;
    toItem.selectionRange.end.line = 1;
    toItem.selectionRange.end.column = 15;
    iter.to = toItem;

    Range fromRange;
    fromRange.start.line = 10;
    fromRange.start.column = 5;
    fromRange.end.line = 10;
    fromRange.end.column = 15;
    iter.fromRanges.push_back(fromRange);

    json reply;
    bool result = ToJSON(iter, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["to"]["name"], "calledFunction");
    ASSERT_EQ(reply["fromRanges"].size(), 1u);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["line"], 10);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["character"], 5);
    EXPECT_EQ(reply["fromRanges"][0]["end"]["line"], 10);
    EXPECT_EQ(reply["fromRanges"][0]["end"]["character"], 15);
}

// Test case for CallHierarchyIncomingCall ToJSON
TEST_F(ProtocolTest001, ToJSON_CallHierarchyIncomingCall_ValidInput) {
    CallHierarchyIncomingCall iter;

    CallHierarchyItem fromItem;
    fromItem.name = "callingFunction";
    fromItem.kind = SymbolKind::FUNCTION;
    fromItem.uri.file = "file:///caller.cj";
    fromItem.range.start.line = 0;
    fromItem.range.start.column = 0;
    fromItem.range.end.line = 5;
    fromItem.range.end.column = 20;
    fromItem.selectionRange.start.line = 1;
    fromItem.selectionRange.start.column = 5;
    fromItem.selectionRange.end.line = 1;
    fromItem.selectionRange.end.column = 15;
    iter.from = fromItem;

    Range fromRange;
    fromRange.start.fileID = 1;
    fromRange.start.line = 10;
    fromRange.start.column = 5;
    fromRange.end.fileID = 1;
    fromRange.end.line = 10;
    fromRange.end.column = 15;
    iter.fromRanges.push_back(fromRange);

    json reply;
    bool result = ToJSON(iter, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["from"]["name"], "callingFunction");
    ASSERT_EQ(reply["fromRanges"].size(), 1u);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["fileID"], 1);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["line"], 10);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["character"], 5);
    EXPECT_EQ(reply["fromRanges"][0]["end"]["fileID"], 1);
    EXPECT_EQ(reply["fromRanges"][0]["end"]["line"], 10);
    EXPECT_EQ(reply["fromRanges"][0]["end"]["character"], 15);
}

// Test case for CodeAction ToJSON
TEST_F(ProtocolTest001, ToJSON_CodeAction_ValidInput) {
    CodeAction params;
    params.title = "Fix import";
    params.kind = CodeAction::QUICKFIX_KIND;

    DiagnosticToken diag;
    diag.range.start.line = 5;
    diag.range.start.column = 10;
    diag.range.end.line = 5;
    diag.range.end.column = 20;
    diag.severity = 1;
    diag.source = "compiler";
    diag.message = "Import missing";
    params.diagnostics = std::vector{diag};

    WorkspaceEdit edit;
    TextEdit textEdit;
    textEdit.range.start.line = 0;
    textEdit.range.start.column = 0;
    textEdit.range.end.line = 0;
    textEdit.range.end.column = 0;
    textEdit.newText = "import com.example;\n";
    edit.changes["file:///test.cj"] = std::vector{textEdit};
    params.edit = edit;

    Command command;
    command.title = "Run formatter";
    command.command = "format.code";
    params.command = command;

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    EXPECT_EQ(reply["title"], "Fix import");
    EXPECT_EQ(reply["kind"], CodeAction::QUICKFIX_KIND);
    ASSERT_TRUE(reply.contains("diagnostics"));
    ASSERT_EQ(reply["diagnostics"].size(), 1u);
    EXPECT_EQ(reply["diagnostics"][0]["message"], "Import missing");
    ASSERT_TRUE(reply.contains("edit"));
    ASSERT_TRUE(reply["edit"]["changes"].contains("file:///test.cj"));
    EXPECT_EQ(reply["edit"]["changes"]["file:///test.cj"][0]["newText"], "import com.example;\n");
    ASSERT_TRUE(reply.contains("command"));
    EXPECT_EQ(reply["command"]["title"], "Run formatter");
    EXPECT_EQ(reply["command"]["command"], "format.code");
}

// Test case for ApplyWorkspaceEditParams ToJSON
TEST_F(ProtocolTest001, ToJSON_ApplyWorkspaceEditParams_ValidInput) {
    ApplyWorkspaceEditParams params;

    TextEdit edit;
    edit.range.start.line = 0;
    edit.range.start.column = 0;
    edit.range.end.line = 0;
    edit.range.end.column = 10;
    edit.newText = "new content";
    params.edit.changes["file:///test.cj"] = std::vector{edit};

    json reply;
    bool result = ToJSON(params, reply);

    EXPECT_TRUE(result);
    ASSERT_TRUE(reply.contains("edit"));
    ASSERT_TRUE(reply["edit"]["changes"].contains("file:///test.cj"));
    EXPECT_EQ(reply["edit"]["changes"]["file:///test.cj"][0]["newText"], "new content");
}

// Test case for FileRefactorRespParams ToJSON
TEST_F(ProtocolTest001, ToJSON_FileRefactorRespParams_ValidInput) {
    FileRefactorRespParams item;

    FileRefactorChange change;
    change.type = FileRefactorChangeType::CHANGED;
    change.range.start.line = 0;
    change.range.start.column = 0;
    change.range.end.line = 0;
    change.range.end.column = 10;
    change.content = "new content";

    item.changes["file:///test.cj"].insert(change);

    json reply;
    bool result = ToJSON(item, reply);

    EXPECT_TRUE(result);
    ASSERT_TRUE(reply.contains("changes"));
    ASSERT_TRUE(reply["changes"].contains("file:///test.cj"));
    ASSERT_EQ(reply["changes"]["file:///test.cj"].size(), 1u);
    EXPECT_EQ(reply["changes"]["file:///test.cj"][0]["type"], static_cast<int>(FileRefactorChangeType::CHANGED));
    EXPECT_EQ(reply["changes"]["file:///test.cj"][0]["content"], "new content");
}

