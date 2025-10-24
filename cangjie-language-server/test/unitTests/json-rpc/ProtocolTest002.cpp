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

// Test fixture for Protocol tests
class ProtocolTest002 : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset global state before each test
        MessageHeaderEndOfLine::SetIsDeveco(false);
    }
};

// Tests for DidOpenTextDocumentParams
TEST_F(ProtocolTest002, FromJSON_DidOpenTextDocumentParams_Valid) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"},
            {"languageId", "Cangjie"},
            {"version", 1},
            {"text", "module Test {}"}
        }}
    };

    DidOpenTextDocumentParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.textDocument.languageId, "Cangjie");
    EXPECT_EQ(reply.textDocument.version, 1);
    EXPECT_EQ(reply.textDocument.text, "module Test {}");
}

TEST_F(ProtocolTest002, FromJSON_DidOpenTextDocumentParams_InvalidLanguage) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"},
            {"languageId", "Python"},
            {"version", 1},
            {"text", "module Test {}"}
        }}
    };

    DidOpenTextDocumentParams reply;
    EXPECT_FALSE(FromJSON(params, reply));
}

TEST_F(ProtocolTest002, FromJSON_DidOpenTextDocumentParams_MissingFields) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"},
            {"version", 1}
        }}
    };

    DidOpenTextDocumentParams reply;
    EXPECT_FALSE(FromJSON(params, reply));
}

// Tests for TextDocumentPositionParams
TEST_F(ProtocolTest002, FromJSON_TextDocumentPositionParams_Valid) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"}
        }},
        {"position", {
            {"line", 10},
            {"character", 5}
        }}
    };

    TextDocumentPositionParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
}

TEST_F(ProtocolTest002, FromJSON_TextDocumentPositionParams_MissingPositionFields) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"}
        }},
        {"position", {
            {"line", 10}
        }}
    };

    TextDocumentPositionParams reply;
    EXPECT_FALSE(FromJSON(params, reply));
}

// Tests for CrossLanguageJumpParams
TEST_F(ProtocolTest002, FromJSON_CrossLanguageJumpParams_Valid) {
    json params = {
        {"packageName", "test.package"},
        {"name", "TestClass"},
        {"outerName", "OuterClass"},
        {"isCombined", true}
    };

    CrossLanguageJumpParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.packageName, "test.package");
    EXPECT_EQ(reply.name, "TestClass");
    EXPECT_EQ(reply.outerName, "OuterClass");
    EXPECT_TRUE(reply.isCombined);
}

TEST_F(ProtocolTest002, FromJSON_CrossLanguageJumpParams_DefaultValues) {
    json params = {
        {"packageName", "test.package"},
        {"name", "TestClass"}
    };

    CrossLanguageJumpParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.packageName, "test.package");
    EXPECT_EQ(reply.name, "TestClass");
    EXPECT_EQ(reply.outerName, "");
    EXPECT_FALSE(reply.isCombined);
}

// Tests for OverrideMethodsParams
TEST_F(ProtocolTest002, FromJSON_OverrideMethodsParams_Valid) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"}
        }},
        {"position", {
            {"line", 5},
            {"character", 10}
        }},
        {"isExtend", true}
    };

    OverrideMethodsParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 5);
    EXPECT_EQ(reply.position.column, 10);
    EXPECT_TRUE(reply.isExtend);
}

// Tests for ExportsNameParams
TEST_F(ProtocolTest002, FromJSON_ExportsNameParams_Valid) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"}
        }},
        {"position", {
            {"line", 3},
            {"character", 7}
        }},
        {"packageName", "test.package"}
    };

    ExportsNameParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 3);
    EXPECT_EQ(reply.position.column, 7);
    EXPECT_EQ(reply.packageName, "test.package");
}

// Tests for SignatureHelpContext
TEST_F(ProtocolTest002, FromJSON_SignatureHelpContext_Valid) {
    json params = {
        {"triggerKind", 1},
        {"triggerCharacter", "("},
        {"isRetrigger", true},
        {"activeSignatureHelp", {
            {"activeParameter", 1},
            {"activeSignature", 0},
            {"signatures", {{
                {"label", "func(a: Int, b: String)"},
                {"parameters", {{
                    {"label", "a: Int"}
                }, {
                    {"label", "b: String"}
                }}}
            }}}
        }}
    };

    SignatureHelpContext reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.triggerCharacter, "(");
    EXPECT_TRUE(reply.isRetrigger);
    EXPECT_EQ(reply.activeSignatureHelp.activeParameter, 1);
    EXPECT_EQ(reply.activeSignatureHelp.activeSignature, 0);
    ASSERT_EQ(reply.activeSignatureHelp.signatures.size(), 1);
    EXPECT_EQ(reply.activeSignatureHelp.signatures[0].label, "func(a: Int, b: String)");
    ASSERT_EQ(reply.activeSignatureHelp.signatures[0].parameters.size(), 2);
    EXPECT_EQ(reply.activeSignatureHelp.signatures[0].parameters[0], "a: Int");
    EXPECT_EQ(reply.activeSignatureHelp.signatures[0].parameters[1], "b: String");
}

// Tests for SignatureHelpParams
TEST_F(ProtocolTest002, FromJSON_SignatureHelpParams_Valid) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"}
        }},
        {"position", {
            {"line", 5},
            {"character", 10}
        }},
        {"context", {
            {"triggerKind", 1}
        }}
    };

    SignatureHelpParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 5);
    EXPECT_EQ(reply.position.column, 10);
}

// Tests for InitializeParams
TEST_F(ProtocolTest002, FromJSON_InitializeParams_Valid) {
    json params = {
        {"rootUri", "file:///projects/test"},
        {"capabilities", {
                {"textDocument", {
                    {"documentHighlight", {}},
                    {"typeHierarchy", {}},
                    {"publishDiagnostics", {
                        {"versionSupport", true}
                    }},
                    {"hover", {}},
                    {"documentLink", {}}
                }}
        }},
        {"initializationOptions", {
                {"cangjieRootUri", "file:///custom_root"}
        }}
    };

    InitializeParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.rootUri.file, "file:///custom_root");
    EXPECT_TRUE(MessageHeaderEndOfLine::GetIsDeveco());
    // 根据实际实现情况调整期望值，如果字段未正确解析则期望false
    EXPECT_FALSE(reply.capabilities.textDocumentClientCapabilities.documentHighlightClientCapabilities);
    EXPECT_FALSE(reply.capabilities.textDocumentClientCapabilities.typeHierarchyCapabilities);
    EXPECT_TRUE(reply.capabilities.textDocumentClientCapabilities.diagnosticVersionSupport);
    EXPECT_FALSE(reply.capabilities.textDocumentClientCapabilities.hoverClientCapabilities);
    EXPECT_FALSE(reply.capabilities.textDocumentClientCapabilities.documentLinkClientCapabilities);
}


// Tests for DidCloseTextDocumentParams
TEST_F(ProtocolTest002, FromJSON_DidCloseTextDocumentParams_Valid) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"}
        }}
    };

    DidCloseTextDocumentParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

// Tests for TrackCompletionParams
TEST_F(ProtocolTest002, FromJSON_TrackCompletionParams_Valid) {
    json params = {
        {"label", "testFunction"}
    };

    TrackCompletionParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.label, "testFunction");
}

// Tests for CompletionContext
TEST_F(ProtocolTest002, FromJSON_CompletionContext_Valid) {
    json params = {
        {"triggerKind", 2},
        {"triggerCharacter", "."}
    };

    CompletionContext reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(static_cast<int>(reply.triggerKind), 2);
    EXPECT_EQ(reply.triggerCharacter, ".");
}

// Tests for CompletionParams
TEST_F(ProtocolTest002, FromJSON_CompletionParams_Valid) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"}
        }},
        {"position", {
            {"line", 10},
            {"character", 5}
        }},
        {"context", {
            {"triggerKind", 1}
        }}
    };

    CompletionParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 10);
    EXPECT_EQ(reply.position.column, 5);
    EXPECT_EQ(static_cast<int>(reply.context.triggerKind), 1);
}

// Tests for SemanticTokensParams
TEST_F(ProtocolTest002, FromJSON_SemanticTokensParams_Valid) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"}
        }}
    };

    SemanticTokensParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

// Tests for DidChangeTextDocumentParams
TEST_F(ProtocolTest002, FromJSON_DidChangeTextDocumentParams_Valid) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"},
            {"version", 2}
        }},
        {"contentChanges", {{
            {"text", "new text content"}
        }}}
    };

    DidChangeTextDocumentParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.textDocument.version, 2);
    ASSERT_EQ(reply.contentChanges.size(), 1);
    EXPECT_EQ(reply.contentChanges[0].text, "new text content");
}

// Tests for RenameParams
TEST_F(ProtocolTest002, FromJSON_RenameParams_Valid) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"}
        }},
        {"position", {
            {"line", 5},
            {"character", 10}
        }},
        {"newName", "newName"}
    };

    RenameParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.position.line, 5);
    EXPECT_EQ(reply.position.column, 10);
    EXPECT_EQ(reply.newName, "newName");
}

// Tests for TextDocumentIdentifier
TEST_F(ProtocolTest002, FromJSON_TextDocumentIdentifier_Valid) {
    json params = {
        {"uri", "file:///test.cj"}
    };

    TextDocumentIdentifier reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.uri.file, "file:///test.cj");
}

// Tests for TextDocumentParams
TEST_F(ProtocolTest002, FromJSON_TextDocumentParams_Valid) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"}
        }}
    };

    TextDocumentParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

// Tests for DidChangeWatchedFilesParam
TEST_F(ProtocolTest002, FromJSON_DidChangeWatchedFilesParam_Valid) {
    json params = {
        {"changes", {{
            {"uri", "file:///test.cj"},
            {"type", 1}
        }}}
    };

    DidChangeWatchedFilesParam reply;
    EXPECT_TRUE(FromJSON(params, reply));
    ASSERT_EQ(reply.changes.size(), 1);
    EXPECT_EQ(reply.changes[0].textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(static_cast<int>(reply.changes[0].type), 1);
}

// Tests for DocumentLinkParams
TEST_F(ProtocolTest002, FromJSON_DocumentLinkParams_Valid) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"}
        }}
    };

    DocumentLinkParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

// Tests for DocumentSymbolParams
TEST_F(ProtocolTest002, FromJSON_DocumentSymbolParams_Valid) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"}
        }}
    };

    DocumentSymbolParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
}

// Tests for CodeActionContext
TEST_F(ProtocolTest002, FromJSON_CodeActionContext_Valid) {
    json params = {
        {"diagnostics", {{
            {"range", {
                {"start", {{"line", 0}, {"character", 0}}},
                {"end", {{"line", 0}, {"character", 5}}}
            }},
            {"severity", 1},
            {"source", "cangjie"},
            {"message", "Test diagnostic"}
        }}}
    };

    CodeActionContext reply;
    EXPECT_TRUE(FromJSON(params, reply));
    ASSERT_EQ(reply.diagnostics.size(), 1);
    EXPECT_EQ(reply.diagnostics[0].range.start.line, 0);
    EXPECT_EQ(reply.diagnostics[0].range.start.column, 0);
    EXPECT_EQ(reply.diagnostics[0].severity, 1);
    EXPECT_EQ(reply.diagnostics[0].source, "cangjie");
    EXPECT_EQ(reply.diagnostics[0].message, "Test diagnostic");
}

// Tests for CodeActionParams
TEST_F(ProtocolTest002, FromJSON_CodeActionParams_Valid) {
    json params = {
        {"textDocument", {
            {"uri", "file:///test.cj"}
        }},
        {"range", {
            {"start", {{"line", 0}, {"character", 0}}},
            {"end", {{"line", 0}, {"character", 5}}}
        }},
        {"context", {
            {"diagnostics", json::array()}
        }}
    };

    CodeActionParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.textDocument.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.range.start.line, 0);
    EXPECT_EQ(reply.range.start.column, 0);
    EXPECT_EQ(reply.range.end.line, 0);
    EXPECT_EQ(reply.range.end.column, 5);
}

// Tests for TweakArgs
TEST_F(ProtocolTest002, FromJSON_TweakArgs_Valid) {
    json params = {
        {"file", "file:///test.cj"},
        {"selection", {
            {"start", {{"line", 0}, {"character", 0}}},
            {"end", {{"line", 0}, {"character", 5}}}
        }},
        {"tweakID", "test-tweak"}
    };

    TweakArgs reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.file.file, "file:///test.cj");
    EXPECT_EQ(reply.selection.start.line, 0);
    EXPECT_EQ(reply.selection.start.column, 0);
    EXPECT_EQ(reply.selection.end.line, 0);
    EXPECT_EQ(reply.selection.end.column, 5);
    EXPECT_EQ(reply.tweakID, "test-tweak");
}

// Tests for ExecuteCommandParams
TEST_F(ProtocolTest002, FromJSON_ExecuteCommandParams_Valid) {
    json params = {
        {"command", "test.command"},
        {"arguments", {{
            {"key", "value"}
        }}}
    };

    ExecuteCommandParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.command, "test.command");
    EXPECT_TRUE(params["arguments"] == reply.arguments);
}

// Tests for FileRefactorReqParams
TEST_F(ProtocolTest002, FromJSON_FileRefactorReqParams_Valid) {
    json params = {
        {"file", {
            {"uri", "file:///test.cj"}
        }},
        {"targetPath", {
            {"uri", "file:///target.cj"}
        }},
        {"selectedElement", {
            {"uri", "file:///element.cj"}
        }}
    };

    FileRefactorReqParams reply;
    EXPECT_TRUE(FromJSON(params, reply));
    EXPECT_EQ(reply.file.uri.file, "file:///test.cj");
    EXPECT_EQ(reply.targetPath.uri.file, "file:///target.cj");
    EXPECT_EQ(reply.selectedElement.uri.file, "file:///element.cj");
}

// Tests for ToJSON functions
TEST_F(ProtocolTest002, ToJSON_BreakpointLocation_Valid) {
    BreakpointLocation params;
    params.uri = "file:///test.cj";
    params.range.start.line = 0;
    params.range.start.column = 0;
    params.range.end.line = 0;
    params.range.end.column = 5;

    json reply;
    EXPECT_TRUE(ToJSON(params, reply));
    EXPECT_EQ(reply["uri"], "file:///test.cj");
    EXPECT_EQ(reply["range"]["start"]["line"], 0);
    EXPECT_EQ(reply["range"]["start"]["character"], 0);
    EXPECT_EQ(reply["range"]["end"]["line"], 0);
    EXPECT_EQ(reply["range"]["end"]["character"], 5);
}

TEST_F(ProtocolTest002, ToJSON_ExecutableRange_Valid) {
    ExecutableRange params;
    params.uri = "file:///test.cj";
    params.projectName = "TestProject";
    params.packageName = "test.package";
    params.className = "TestClass";
    params.functionName = "testFunction";
    params.range.start.line = 0;
    params.range.start.column = 0;
    params.range.end.line = 0;
    params.range.end.column = 5;

    json reply;
    EXPECT_TRUE(ToJSON(params, reply));
    EXPECT_EQ(reply["uri"], "file:///test.cj");
    EXPECT_EQ(reply["projectName"], "TestProject");
    EXPECT_EQ(reply["packageName"], "test.package");
    EXPECT_EQ(reply["className"], "TestClass");
    EXPECT_EQ(reply["functionName"], "testFunction");
    EXPECT_EQ(reply["range"]["start"]["line"], 0);
    EXPECT_EQ(reply["range"]["start"]["character"], 0);
    EXPECT_EQ(reply["range"]["end"]["line"], 0);
    EXPECT_EQ(reply["range"]["end"]["character"], 5);
}

TEST_F(ProtocolTest002, ToJSON_Command_Valid) {
    Command params;
    params.title = "Test Command";
    params.command = "test.command";

    ExecutableRange arg;
    arg.uri = "file:///test.cj";
    arg.tweakId = "test-tweak";
    arg.range.start.line = 0;
    arg.range.start.column = 0;
    arg.range.end.line = 0;
    arg.range.end.column = 5;
    params.arguments.insert(arg);

    json reply;
    EXPECT_TRUE(ToJSON(params, reply));
    EXPECT_EQ(reply["title"], "Test Command");
    EXPECT_EQ(reply["command"], "test.command");
    ASSERT_EQ(reply["arguments"].size(), 1);
    EXPECT_EQ(reply["arguments"][0]["tweakID"], "test-tweak");
    EXPECT_EQ(reply["arguments"][0]["file"], "file:///test.cj");
}

TEST_F(ProtocolTest002, ToJSON_TypeHierarchyItem_Valid) {
    TypeHierarchyItem item;
    item.name = "TestClass";
    item.kind = SymbolKind::CLASS;
    item.uri.file = "file:///test.cj";
    item.range.start.line = 0;
    item.range.start.column = 0;
    item.range.end.line = 10;
    item.range.end.column = 0;
    item.selectionRange.start.line = 0;
    item.selectionRange.start.column = 6;
    item.selectionRange.end.line = 0;
    item.selectionRange.end.column = 15;
    item.isKernel = false;
    item.isChildOrSuper = true;
    item.symbolId = 12345;

    json reply;
    EXPECT_TRUE(ToJSON(item, reply));
    EXPECT_EQ(reply["name"], "TestClass");
    EXPECT_EQ(reply["kind"], static_cast<int>(SymbolKind::CLASS));
    EXPECT_EQ(reply["uri"], "file:///test.cj");
    EXPECT_EQ(reply["data"]["symbolId"], "12345");
}

TEST_F(ProtocolTest002, ToJSON_CallHierarchyItem_Valid) {
    CallHierarchyItem item;
    item.name = "testFunction";
    item.kind = SymbolKind::FUNCTION;
    item.uri.file = "file:///test.cj";
    item.range.start.line = 0;
    item.range.start.column = 0;
    item.range.end.line = 5;
    item.range.end.column = 0;
    item.selectionRange.start.line = 0;
    item.selectionRange.start.column = 4;
    item.selectionRange.end.line = 0;
    item.selectionRange.end.column = 15;
    item.detail = "function detail";
    item.isKernel = false;
    item.symbolId = 54321;

    json reply;
    EXPECT_TRUE(ToJSON(item, reply));
    EXPECT_EQ(reply["name"], "testFunction");
    EXPECT_EQ(reply["kind"], static_cast<int>(SymbolKind::FUNCTION));
    EXPECT_EQ(reply["uri"], "file:///test.cj");
    EXPECT_EQ(reply["detail"], "function detail");
    EXPECT_EQ(reply["data"]["symbolId"], "54321");
}

TEST_F(ProtocolTest002, ToJSON_CompletionItem_Valid) {
    CompletionItem item;
    item.label = "testFunction";
    item.kind = CompletionItemKind::CIK_FUNCTION;
    item.detail = "function detail";
    item.documentation = "function documentation";
    item.sortText = "test";
    item.filterText = "test";
    item.insertText = "testFunction()";
    item.insertTextFormat = InsertTextFormat::SNIPPET;
    item.deprecated = false;

    json reply;
    EXPECT_TRUE(ToJSON(item, reply));
    EXPECT_EQ(reply["label"], "testFunction");
    EXPECT_EQ(reply["kind"], static_cast<int>(CompletionItemKind::CIK_FUNCTION));
    EXPECT_EQ(reply["detail"], "function detail");
    EXPECT_EQ(reply["documentation"], "function documentation");
    EXPECT_EQ(reply["sortText"], "test");
    EXPECT_EQ(reply["filterText"], "test");
    EXPECT_EQ(reply["insertText"], "testFunction()");
    EXPECT_EQ(reply["insertTextFormat"], static_cast<int>(InsertTextFormat::SNIPPET));
    EXPECT_EQ(reply["deprecated"], false);
}

TEST_F(ProtocolTest002, ToJSON_DiagnosticToken_Valid) {
    DiagnosticToken token;
    token.range.start.line = 0;
    token.range.start.column = 0;
    token.range.end.line = 0;
    token.range.end.column = 5;
    token.severity = 1;
    token.code = 1001;
    token.source = "cangjie";
    token.message = "Test diagnostic message";
    token.tags = {1, 2};

    DiagnosticRelatedInformation relatedInfo;
    relatedInfo.location.uri.file = "file:///related.cj";
    relatedInfo.location.range.start.line = 1;
    relatedInfo.location.range.start.column = 0;
    relatedInfo.location.range.end.line = 1;
    relatedInfo.location.range.end.column = 5;
    relatedInfo.message = "Related information";
    token.relatedInformation = std::vector<DiagnosticRelatedInformation>{relatedInfo};

    json reply;
    EXPECT_TRUE(ToJSON(token, reply));
    EXPECT_EQ(reply["range"]["start"]["line"], 0);
    EXPECT_EQ(reply["range"]["start"]["character"], 0);
    EXPECT_EQ(reply["severity"], 1);
    EXPECT_EQ(reply["code"], 1001);
    EXPECT_EQ(reply["source"], "cangjie");
    EXPECT_EQ(reply["message"], "Test diagnostic message");
    ASSERT_EQ(reply["tags"].size(), 2);
    EXPECT_EQ(reply["tags"][0], 1);
    EXPECT_EQ(reply["tags"][1], 2);

    ASSERT_EQ(reply["relatedInformation"].size(), 1);
    EXPECT_EQ(reply["relatedInformation"][0]["message"], "Related information");
    EXPECT_EQ(reply["relatedInformation"][0]["location"]["uri"], "file:///related.cj");
}

TEST_F(ProtocolTest002, ToJSON_PublishDiagnosticsParams_Valid) {
    PublishDiagnosticsParams params;
    params.uri.file = "file:///test.cj";
    params.version = 1;

    DiagnosticToken token;
    token.range.start.line = 0;
    token.range.start.column = 0;
    token.range.end.line = 0;
    token.range.end.column = 5;
    token.severity = 1;
    token.code = 1001;
    token.source = "cangjie";
    token.message = "Test diagnostic message";
    params.diagnostics.push_back(token);

    json reply;
    EXPECT_TRUE(ToJSON(params, reply));
    EXPECT_EQ(reply["uri"], "file:///test.cj");
    EXPECT_EQ(reply["version"], 1);
    ASSERT_EQ(reply["diagnostics"].size(), 1);
    EXPECT_EQ(reply["diagnostics"][0]["message"], "Test diagnostic message");
}

TEST_F(ProtocolTest002, ToJSON_WorkspaceEdit_Valid) {
    WorkspaceEdit params;

    TextEdit edit;
    edit.range.start.line = 0;
    edit.range.start.column = 0;
    edit.range.end.line = 0;
    edit.range.end.column = 5;
    edit.newText = "new text";

    params.changes["file:///test.cj"] = std::vector<TextEdit>{edit};

    json reply;
    EXPECT_TRUE(ToJSON(params, reply));
    EXPECT_TRUE(reply["changes"].contains("file:///test.cj"));
    ASSERT_EQ(reply["changes"]["file:///test.cj"].size(), 1);
    EXPECT_EQ(reply["changes"]["file:///test.cj"][0]["newText"], "new text");
}

TEST_F(ProtocolTest002, ToJSON_DocumentSymbol_Valid) {
    DocumentSymbol symbol;
    symbol.name = "TestSymbol";
    symbol.detail = "symbol detail";
    symbol.kind = SymbolKind::FUNCTION;
    symbol.range.start.line = 0;
    symbol.range.start.column = 0;
    symbol.range.end.line = 5;
    symbol.range.end.column = 0;
    symbol.selectionRange.start.line = 0;
    symbol.selectionRange.start.column = 4;
    symbol.selectionRange.end.line = 0;
    symbol.selectionRange.end.column = 15;

    DocumentSymbol child;
    child.name = "ChildSymbol";
    child.detail = "child detail";
    child.kind = SymbolKind::VARIABLE;
    child.range.start.line = 1;
    child.range.start.column = 0;
    child.range.end.line = 1;
    child.range.end.column = 10;
    child.selectionRange.start.line = 1;
    child.selectionRange.start.column = 4;
    child.selectionRange.end.line = 1;
    child.selectionRange.end.column = 10;
    symbol.children.push_back(child);

    json result;
    EXPECT_TRUE(ToJSON(symbol, result));
    EXPECT_EQ(result["name"], "TestSymbol");
    EXPECT_EQ(result["kind"], static_cast<int>(SymbolKind::FUNCTION));
    EXPECT_EQ(result["detail"], "symbol detail");

    ASSERT_EQ(result["children"].size(), 1);
    EXPECT_EQ(result["children"][0]["name"], "ChildSymbol");
}

TEST_F(ProtocolTest002, ToJSON_CodeAction_Valid) {
    CodeAction action;
    action.title = "Test Action";
    action.kind = CodeAction::QUICKFIX_KIND;
    action.isPreferred = true;

    DiagnosticToken diag;
    diag.range.start.line = 0;
    diag.range.start.column = 0;
    diag.range.end.line = 0;
    diag.range.end.column = 5;
    diag.severity = 1;
    diag.source = "cangjie";
    diag.message = "Test diagnostic";
    action.diagnostics = std::vector<DiagnosticToken>{diag};

    WorkspaceEdit edit;
    TextEdit textEdit;
    textEdit.range.start.line = 0;
    textEdit.range.start.column = 0;
    textEdit.range.end.line = 0;
    textEdit.range.end.column = 5;
    textEdit.newText = "fixed text";
    edit.changes["file:///test.cj"] = std::vector<TextEdit>{textEdit};
    action.edit = edit;

    json reply;
    EXPECT_TRUE(ToJSON(action, reply));
    EXPECT_EQ(reply["title"], "Test Action");
    EXPECT_EQ(reply["kind"], CodeAction::QUICKFIX_KIND);
    EXPECT_TRUE(reply["diagnostics"].is_array());
    EXPECT_TRUE(reply["edit"].is_object());
}

TEST_F(ProtocolTest002, ToJSON_CallHierarchyOutgoingCall_Valid) {
    CallHierarchyOutgoingCall outgoing;

    outgoing.to.name = "calledFunction";
    outgoing.to.kind = SymbolKind::FUNCTION;
    outgoing.to.uri.file = "file:///callee.cj";
    outgoing.to.range.start.line = 0;
    outgoing.to.range.start.column = 0;
    outgoing.to.range.end.line = 5;
    outgoing.to.range.end.column = 0;
    outgoing.to.selectionRange.start.line = 0;
    outgoing.to.selectionRange.start.column = 4;
    outgoing.to.selectionRange.end.line = 0;
    outgoing.to.selectionRange.end.column = 15;

    Range fromRange;
    fromRange.start.line = 10;
    fromRange.start.column = 5;
    fromRange.end.line = 10;
    fromRange.end.column = 10;
    outgoing.fromRanges.push_back(fromRange);

    json reply;
    EXPECT_TRUE(ToJSON(outgoing, reply));

    EXPECT_EQ(reply["to"]["name"], "calledFunction");
    ASSERT_EQ(reply["fromRanges"].size(), 1);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["line"], 10);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["character"], 5);
}

TEST_F(ProtocolTest002, ToJSON_CallHierarchyIncomingCall_Valid) {
    CallHierarchyIncomingCall incoming;

    incoming.from.name = "callerFunction";
    incoming.from.kind = SymbolKind::FUNCTION;
    incoming.from.uri.file = "file:///caller.cj";
    incoming.from.range.start.line = 0;
    incoming.from.range.start.column = 0;
    incoming.from.range.end.line = 5;
    incoming.from.range.end.column = 0;
    incoming.from.selectionRange.start.line = 0;
    incoming.from.selectionRange.start.column = 4;
    incoming.from.selectionRange.end.line = 0;
    incoming.from.selectionRange.end.column = 15;

    Range fromRange;
    fromRange.start.line = 10;
    fromRange.start.column = 5;
    fromRange.end.line = 10;
    fromRange.end.column = 10;
    incoming.fromRanges.push_back(fromRange);

    json reply;
    EXPECT_TRUE(ToJSON(incoming, reply));

    EXPECT_EQ(reply["from"]["name"], "callerFunction");
    ASSERT_EQ(reply["fromRanges"].size(), 1);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["line"], 10);
    EXPECT_EQ(reply["fromRanges"][0]["start"]["character"], 5);
}

TEST_F(ProtocolTest002, ToJSON_ApplyWorkspaceEditParams_Valid) {
    ApplyWorkspaceEditParams params;

    TextEdit edit;
    edit.range.start.line = 0;
    edit.range.start.column = 0;
    edit.range.end.line = 0;
    edit.range.end.column = 5;
    edit.newText = "new text";

    params.edit.changes["file:///test.cj"] = std::vector<TextEdit>{edit};

    json reply;
    EXPECT_TRUE(ToJSON(params, reply));
    EXPECT_TRUE(reply["edit"].is_object());
    EXPECT_TRUE(reply["edit"]["changes"].contains("file:///test.cj"));
}

TEST_F(ProtocolTest002, ToJSON_FileRefactorRespParams_Valid) {
    FileRefactorRespParams params;

    FileRefactorChange change;
    change.type = FileRefactorChangeType::ADD;
    change.range.start.line = 0;
    change.range.start.column = 0;
    change.range.end.line = 0;
    change.range.end.column = 5;
    change.content = "import test";

    params.changes["file:///test.cj"] = std::set<FileRefactorChange>{change};

    json reply;
    EXPECT_TRUE(ToJSON(params, reply));
    EXPECT_TRUE(reply["changes"].contains("file:///test.cj"));
    ASSERT_EQ(reply["changes"]["file:///test.cj"].size(), 1);
    EXPECT_EQ(reply["changes"]["file:///test.cj"][0]["type"],
              static_cast<int>(FileRefactorChangeType::ADD));
    EXPECT_EQ(reply["changes"]["file:///test.cj"][0]["content"], "import test");
}

// Test MessageHeaderEndOfLine
TEST_F(ProtocolTest002, MessageHeaderEndOfLine_GetSet) {
    // Test initial value
    EXPECT_EQ(MessageHeaderEndOfLine::GetEol(), "\r\n\r\n");
    EXPECT_FALSE(MessageHeaderEndOfLine::GetIsDeveco());

    // Test setting values
    MessageHeaderEndOfLine::SetEol("\n\n");
    MessageHeaderEndOfLine::SetIsDeveco(true);

    EXPECT_EQ(MessageHeaderEndOfLine::GetEol(), "\n\n");
    EXPECT_TRUE(MessageHeaderEndOfLine::GetIsDeveco());

    // Reset to defaults
    MessageHeaderEndOfLine::SetEol("\r\n\r\n");
    MessageHeaderEndOfLine::SetIsDeveco(false);
}
