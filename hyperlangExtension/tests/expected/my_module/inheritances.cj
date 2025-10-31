// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*
import tpc.AbilityKit.common
/*class  ExportKeyword DeclareKeyword SubInterface {
    p: number;
    p1: number;
    }*/
/*class  ExportKeyword DeclareKeyword FirstContextualKeyword SuperClass {
    p: number;
    }*/
/*class  ExportKeyword DeclareKeyword SubClass {
    p: number;
    }*/
/*class  ExportKeyword DeclareKeyword SuperClass1 {
    p: number;
    }*/
/*class  ExportKeyword DeclareKeyword SubClass1 {
    p: number;
    }*/
/*interface  ExportKeyword DeclareKeyword SuperInterface {
    p: number;
    }*/
/*interface  A {
    p: number;
    }*/
/*interface  B {
    p1: number;
    }*/
/*interface  C {
    f(): void
    }*/
/*interface  D {
    }*/
/*interface  E {
    }*/
/*interface  F {
    g(): void
    }*/
/*interface  ExportKeyword BuglyLogAdapter {
    }*/

/***********OBJECT***********/


public open class SuperInterface {
    
    protected SuperInterface(public var p: Float64) {}
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["p"] = p.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): SuperInterface {
        let obj = input.asObject()
        SuperInterface(
        Float64.fromJSValue(context, obj["p"])
        )
    }
    
}


public open class A {
    
    protected A(public var p: Float64) {}
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["p"] = p.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): A {
        let obj = input.asObject()
        A(
        Float64.fromJSValue(context, obj["p"])
        )
    }
    
}


public open class B <: A {
    
    protected B(p: Float64,
    public var p1: Float64) { super(p) }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["p"] = p.toJSValue(context)
        obj["p1"] = p1.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): B {
        let obj = input.asObject()
        B(
        Float64.fromJSValue(context, obj["p"]),
        Float64.fromJSValue(context, obj["p1"])
        )
    }
    
}


public open class C {
    
    protected C(public var arkts_object: JSObject) {}
    
    
    /**
	 * @brief f(): void
	 */
    public func f(): Unit {
        jsObjApiCall < Unit >( arkts_object, "f", emptyArg)
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): C {
        C(input.asObject())
    }
}


public open class D <: C {
    
    protected D(arkts_object: JSObject) { super(arkts_object) }
    
    
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): D {
        D(input.asObject())
    }
}


public open class E <: A {
    
    protected E(p: Float64) { super(p) }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["p"] = p.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): E {
        let obj = input.asObject()
        E(
        Float64.fromJSValue(context, obj["p"])
        )
    }
    
}


public open class F <: C {
    
    protected F(arkts_object: JSObject) { super(arkts_object) }
    
    
    /**
	 * @brief g(): void
	 */
    public func g(): Unit {
        jsObjApiCall < Unit >( arkts_object, "g", emptyArg)
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): F {
        F(input.asObject())
    }
}


public open class BuglyLogAdapter <: p29 {
    
    protected BuglyLogAdapter(arkts_object: JSObject) { super(arkts_object) }
    
    
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): BuglyLogAdapter {
        BuglyLogAdapter(input.asObject())
    }
}


public class SubInterface <: SuperInterface {
    
    protected SubInterface(arkts_object: JSObject) {}
    
    
    public mut prop p: Float64 {
        get() {
            checkThreadAndCall < Float64 >(getMainContext()) {
                ctx: JSContext => Float64.fromJSValue(ctx, arkts_object["p"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["p"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    public mut prop p1: Float64 {
        get() {
            checkThreadAndCall < Float64 >(getMainContext()) {
                ctx: JSContext => Float64.fromJSValue(ctx, arkts_object["p1"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["p1"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    
    public func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): SubInterface {
        SubInterface(input.asObject())
    }
}


public open class SuperClass {
    
    protected SuperClass(public var arkts_object: JSObject) {}
    
    
    public mut prop p: Float64 {
        get() {
            checkThreadAndCall < Float64 >(getMainContext()) {
                ctx: JSContext => Float64.fromJSValue(ctx, arkts_object["p"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["p"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): SuperClass {
        SuperClass(input.asObject())
    }
}


public class SubClass <: SuperClass {
    
    protected SubClass(arkts_object: JSObject) { super(arkts_object) }
    
    
    public mut prop p: Float64 {
        get() {
            checkThreadAndCall < Float64 >(getMainContext()) {
                ctx: JSContext => Float64.fromJSValue(ctx, arkts_object["p"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["p"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    
    public func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): SubClass {
        SubClass(input.asObject())
    }
}


public open class SuperClass1 {
    
    protected SuperClass1(public var arkts_object: JSObject) {}
    
    
    public mut prop p: Float64 {
        get() {
            checkThreadAndCall < Float64 >(getMainContext()) {
                ctx: JSContext => Float64.fromJSValue(ctx, arkts_object["p"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["p"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): SuperClass1 {
        SuperClass1(input.asObject())
    }
}


public class SubClass1 <: SuperClass1 {
    
    protected SubClass1(arkts_object: JSObject) { super(arkts_object) }
    
    
    public mut prop p: Float64 {
        get() {
            checkThreadAndCall < Float64 >(getMainContext()) {
                ctx: JSContext => Float64.fromJSValue(ctx, arkts_object["p"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["p"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    
    public func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): SubClass1 {
        SubClass1(input.asObject())
    }
}

