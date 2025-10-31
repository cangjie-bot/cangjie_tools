// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*
import std.math.numeric.*
/*class  ExportKeyword UnionTypes {
    t_null: number | null;
    null_t: null | String;
    t1_t2: boolean | String;
    t_undefined: number | undefined;
    undefined: undefined | String;
    t3: number | String | boolean;
    t4: number | String | boolean | null | undefined;
    t5: number | String | boolean | bigint| void;
    getValueOrError(): number | String
    setValue(value: string): number | boolean
    }*/


public enum Union_2_Members < A, B > {
    | A(A)
    | B(B)
}

public enum Union_3_Members < A, B, C > {
    | A(A)
    | B(B)
    | C(C)
}




public class UnionTypes {
    
    protected UnionTypes(var arkts_object: JSObject) {}
    
    // Properties with union types
    public mut prop t_null: Option<Float64> {
        get() {
            checkThreadAndCall < Option<Float64> >(getMainContext()) {
                ctx: JSContext => Option < Float64 >.fromJSValue(ctx, arkts_object["t_null"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["t_null"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop null_t: Option<String> {
        get() {
            checkThreadAndCall < Option<String> >(getMainContext()) {
                ctx: JSContext => Option < String >.fromJSValue(ctx, arkts_object["null_t"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["null_t"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop t1_t2: Union_2_Members<Bool, String> {
        get() {
            checkThreadAndCall < Union_2_Members<Bool, String> >(getMainContext()) {
                ctx: JSContext => Union_2_Members<Bool, String>.fromJSValue(ctx, arkts_object["t1_t2"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["t1_t2"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop t_undefined: Option<Float64> {
        get() {
            checkThreadAndCall < Option<Float64> >(getMainContext()) {
                ctx: JSContext => Option < Float64 >.fromJSValue(ctx, arkts_object["t_undefined"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["t_undefined"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop undefined: Option<String> {
        get() {
            checkThreadAndCall < Option<String> >(getMainContext()) {
                ctx: JSContext => Option < String >.fromJSValue(ctx, arkts_object["undefined"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["undefined"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop t3: Union_3_Members<Float64, String, Bool> {
        get() {
            checkThreadAndCall < Union_3_Members<Float64, String, Bool> >(getMainContext()) {
                ctx: JSContext => Union_3_Members<Float64, String, Bool>.fromJSValue(ctx, arkts_object["t3"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["t3"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop t4: Union_3_Members<Float64, String, Bool> {
        get() {
            checkThreadAndCall < Union_3_Members<Float64, String, Bool> >(getMainContext()) {
                ctx: JSContext => Union_3_Members<Float64, String, Bool>.fromJSValue(ctx, arkts_object["t4"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["t4"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop t5: JSValue/* FIXME: `number | string | boolean | bigint | void` */ {
        get() {
            checkThreadAndCall < JSValue/* FIXME: `number | string | boolean | bigint | void` */ >(getMainContext()) {
                ctx: JSContext => arkts_object["t5"]
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["t5"] = v
            }
        }
        
    }
    
    /**
	 * @brief getValueOrError(): number | String
	 */
	// Method returning a union type
    public func getValueOrError(): Union_2_Members<Float64, String> {
        jsObjApiCall < Union_2_Members<Float64, String> >( arkts_object, "getValueOrError", emptyArg) {
            ctx, info => Union_2_Members<Float64, String>.fromJSValue(ctx, info)
        }
    }
    /**
	 * @brief setValue(value: string): number | boolean
	 */
	// Method with union type parameter
    public func setValue(value: String): Union_2_Members<Float64, Bool> {
        jsObjApiCall < Union_2_Members<Float64, Bool> >( arkts_object, "setValue", { ctx =>[value.toJSValue(ctx)] }) {
            ctx, info => Union_2_Members<Float64, Bool>.fromJSValue(ctx, info)
        }
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): UnionTypes {
        UnionTypes(input.asObject())
    }
}

/***********METHOD***********/
/**
	 * @brief testUnion(s: string): boolean | String
	 */
public func testUnion(s: String): Union_2_Members<Bool, String> {
    hmsGlobalApiCall < Union_2_Members<Bool, String> >( "my_module_unionTypes", "testUnion", { ctx =>[s.toJSValue(ctx)] }) {
        ctx, info => Union_2_Members<Bool, String>.fromJSValue(ctx, info)
    }
}


