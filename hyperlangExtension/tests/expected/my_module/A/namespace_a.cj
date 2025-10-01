// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module.A

import my_module.*
import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

/***********ENUM***********/

/* enum TestEnum {
	 VALUE1,
	 VALUE2
	}*/
public enum TestEnum <: ToString & Equatable < TestEnum > {
    | VALUE1
    | VALUE2 

    func get(): Int32 {
        match(this) {
            case VALUE1 => 0  //todo: please check the value
            case VALUE2 => 1  //todo: please check the value
        }
    }
    
    static func parse(val: Int32): TestEnum {
        match(val) {
            case 0 => VALUE1  //todo: please check the value
            case 1 => VALUE2  //todo: please check the value
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?Int32): ?TestEnum {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        match(this) {
            case VALUE1 => "VALUE1"
            case VALUE2 => "VALUE2"
        }
    }
    
    public override operator func ==(that: TestEnum): Bool {
        match((this, that)) {
            case(VALUE1, VALUE1) => true
            case(VALUE2, VALUE2) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: TestEnum): Bool {
        !(this == that)
    }
}


/***********OBJECT***********/

/*interface B {
    }*/

public open class B {
    
    protected B() {}
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): B {
        let obj = input.asObject()
        B(
        
        )
    }
    
}

/*interface InterfaceWithMethod {
    prop: String;
    method(x: number): void
    }*/

public open class InterfaceWithMethod {
    
    protected InterfaceWithMethod(public var arkts_object: JSObject) {}
    
    
    public mut prop prop: String {
        get() {
            checkThreadAndCall < String >(getMainContext()) {
                ctx: JSContext => String.fromJSValue(ctx, arkts_object["prop"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["prop"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    /**
	 * @brief method(x: number): void
	 */
    public func method(x: Float64): Unit {
        jsObjApiCall < Unit >( arkts_object, "method", { ctx =>[x.toJSValue(ctx)] })
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): InterfaceWithMethod {
        InterfaceWithMethod(input.asObject())
    }
}

/*class TestClass {
    name: String;
    getValue(): number
    }*/

public class TestClass {
    
    protected TestClass(var arkts_object: JSObject) {}
    
    
    public mut prop name: String {
        get() {
            checkThreadAndCall < String >(getMainContext()) {
                ctx: JSContext => String.fromJSValue(ctx, arkts_object["name"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["name"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    /**
	 * @brief getValue(): number
	 */
    public func getValue(): Float64 {
        jsObjApiCall < Float64 >( arkts_object, "getValue", emptyArg)
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): TestClass {
        TestClass(input.asObject())
    }
}

/*class ClassWithMethod {
    method(a: number, b: string): boolean
    }*/

public class ClassWithMethod {
    
    protected ClassWithMethod(var arkts_object: JSObject) {}
    
    
    /**
	 * @brief method(a: number, b: string): boolean
	 */
    public func method(a: Float64, b: String): Bool {
        jsObjApiCall < Bool >( arkts_object, "method", { ctx =>[a.toJSValue(ctx), b.toJSValue(ctx)] })
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): ClassWithMethod {
        ClassWithMethod(input.asObject())
    }
}

