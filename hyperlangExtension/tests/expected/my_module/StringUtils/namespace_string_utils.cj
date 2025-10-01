// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module.StringUtils

import my_module.*
import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

/***********METHOD***********/
/**
	 * @brief add(a: number, b: number): number
	 */
	/**
     * Concatenate two strings (same signature as MathUtils.add)
     */
public func add(a: Float64, b: Float64): Float64 {
    hmsGlobalApiCall < Float64 >( "my_module_namespace", "StringUtils.add", { ctx =>[a.toJSValue(ctx), b.toJSValue(ctx)] })
}


