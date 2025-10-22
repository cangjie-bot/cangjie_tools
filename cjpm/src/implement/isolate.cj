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
    var compileTarget = crossCompileTarget
    var isTargetLinux = compileTarget.contains("linux")
    var isTargetAndroid = compileTarget.contains("android")
    var isTargetWindows = compileTarget.contains("windows")

    if (crossCompileTarget.isEmpty()) {
        compileTarget = targetConfigName
        isTargetLinux = compileTarget.contains("linux")
        isTargetAndroid = compileTarget.contains("android")
        isTargetWindows = compileTarget.contains("windows")
        return ((isTargetLinux && !isTargetAndroid) || isTargetWindows)
    }

    return return ((isTargetLinux && !isTargetAndroid) || isTargetWindows)
}