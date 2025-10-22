// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement
import cjpm.config.*

@When[os == "Linux"]
func supportLto(): Bool {
    return true
}

@When[os != "Linux"]
func supportLto(): Bool {
    return false
}

func isSupportCombine(): Bool {
    var compileTarget = targetConfigName
    var isLinux = compileTarget.contains("linux")
    var isAndroid = compileTarget.contains("android")
    var isWindows = compileTarget.contains("windows")
    return ((isLinux && !isAndroid) || isWindows)
}