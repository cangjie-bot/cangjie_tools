# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

set(UNIT_TESTS_SRC
        json-rpc/ProtocolTest.cpp
        languageserver/capabilities/fileRefactor/FileMoveTest.cpp
        languageserver/capabilities/fileRefactor/FileRefactorTest.cpp
        languageserver/capabilities/overrideMethods/FindOverrideMethodsUtilsTest.cpp
)

include_directories(
        ../../src/json-rpc
        ../../src/languageserver/capabilities/fileRefactor
        ../../src/languageserver/capabilities/overrideMethods
)

add_library(UnitTests OBJECT ${UNIT_TESTS_SRC})