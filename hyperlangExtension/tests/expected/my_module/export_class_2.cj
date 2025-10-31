// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*
/*class  ExportKeyword DeclareKeyword AppSettings {
    getInstance(): AppSettings
    getSetting(key: string): String | number | boolean | undefined
    }*/


public enum Union_3_Members < A, B, C > {
    | A(A)
    | B(B)
    | C(C)
}




public class AppSettings {
    
    protected AppSettings(var arkts_object: JSObject) {}
    
    
    /**
	 * @brief getInstance(): AppSettings
	 */
    public static func getInstance(): AppSettings {
        jsObjApiCall < AppSettings >(getClassConstructorObj("my_module_export_class_2", "AppSettings"),  "getInstance", emptyArg) {
            ctx, info => AppSettings.fromJSValue(ctx, info)
        }
    }
    /**
	 * @brief getSetting(key: string): String | number | boolean | undefined
	 */
    public func getSetting(key: String): Union_3_Members<String, Float64, Bool> {
        jsObjApiCall < Union_3_Members<String, Float64, Bool> >( arkts_object, "getSetting", { ctx =>[key.toJSValue(ctx)] }) {
            ctx, info => Union_3_Members<String, Float64, Bool>.fromJSValue(ctx, info)
        }
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): AppSettings {
        AppSettings(input.asObject())
    }
}

