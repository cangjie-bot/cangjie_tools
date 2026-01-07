// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package FollowLambdaCompletion.test_import.pkg1

public func func111(a: Int64, b: (Int64, String) -> String) {

}

public func func222(a: Int64, b: Unit) {
    ""
}

public class AAA11 {
    public init() {
        
    }

    public init(a!: () -> String) {
        
    }

    public func func111(b: (Int64, String) -> String) {

    }
}

public var var111 = func111

public var var222: (a: Int64, b: Unit) -> String = func222