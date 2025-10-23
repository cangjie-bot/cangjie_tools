# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

set(UNIT_TESTS_SRC
        json-rpc/ProtocolTest001.cpp
        json-rpc/ProtocolTest002.cpp
)

include_directories(
        ../../src/json-rpc
)

add_library(UnitTests OBJECT ${UNIT_TESTS_SRC})