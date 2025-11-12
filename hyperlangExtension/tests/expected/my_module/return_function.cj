// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

/***********OBJECT***********/

/*interface TestInterface {
    method1(): (x: String) => number
    }*/

public open class TestInterface {
    
    protected TestInterface(public var arkts_object: JSObject) { }
    
    
    /**
	 * @brief method1(): (x: String) => number
	 */
    public func method1(): (x: String) -> Float64 {
        jsObjApiCall < (x: String) -> Float64 >( arkts_object, "method1", emptyArg) {
            ctx, info => (x: String) -> Float64.fromJSValue(ctx, info)
        }
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    public open  static func fromJSValue(context: JSContext, input: JSValue): TestInterface {
        TestInterface(input.asObject())
    }
}

/*class DeclareKeyword TestClass {
    method2(): (y: boolean) => String
    }*/

public class TestClass {
    
    protected TestClass(var arkts_object: JSObject) { }
    
    
    /**
	 * @brief method2(): (y: boolean) => String
	 */
    public func method2(): (y: Bool) -> String {
        jsObjApiCall < (y: Bool) -> String >( arkts_object, "method2", emptyArg) {
            ctx, info => (y: Bool) -> String.fromJSValue(ctx, info)
        }
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
     static func fromJSValue(context: JSContext, input: JSValue): TestClass {
        TestClass(input.asObject())
    }
}

/***********METHOD***********/
/**
	 * @brief test1(): (num: number) => String
	 */
	// Test case for function returning function type
public func test1(): (num: Float64) -> String {
    hmsGlobalApiCall < (num: Float64) -> String >( "my_module_returnFunction", "test1", emptyArg) {
        ctx, info => (num: Float64) -> String.fromJSValue(ctx, info)
    }
}

/**
	 * @brief test2(name: string): (id: number) => boolean
	 */
public func test2(name: String): (id: Float64) -> Bool {
    hmsGlobalApiCall < (id: Float64) -> Bool >( "my_module_returnFunction", "test2", { ctx =>[name.toJSValue(ctx)] }) {
        ctx, info => (id: Float64) -> Bool.fromJSValue(ctx, info)
    }
}

/**
	 * @brief test3(): () => void
	 */
public func test3(): () -> Unit {
    hmsGlobalApiCall < () -> Unit >( "my_module_returnFunction", "test3", emptyArg) {
        ctx, info => () -> Unit.fromJSValue(ctx, info)
    }
}


