package ohos

import ohos.ark_interop.JSModule
import ohos.ark_interop.JSContext
import ohos.ark_interop.JSCallInfo
import ohos.ark_interop.JSValue
import ohos.ark_interop.*
import ohos.ark_interop.JSLambda
import ohos.ark_interop.JSClass

// findUsage 5.1
func testCJ(runtime: JSContext, callInfo: JSCallInfo): JSValue {
    let result = "Hello ${callInfo[0].toString()}"
    runtime.string(result).toJSValue()
}

let EXPORT_MODULE = JSModule.registerModule {
    runtime, exports => exports["testCJ"] = runtime.function(testCJ).toJSValue()
}

// findUsage 5.2
func testFn1801(runtime: JSContext, callInfo: JSCallInfo): JSValue {
    let result = "Hello ${callInfo[0].toString()}"
    runtime.string(result).toJSValue()
}

func xxx(runtime: JSContext, exports:JSObject): Unit {
    exports["testFn18"] = runtime.function(testFn1801).toJSValue()
}
var tmp18 = JSModule.registerModule(xxx)

// findUsage 6.1
func hello(runtime: JSContext, callInfo: JSCallInfo): JSValue {
    let result = "Hello ${callInfo[0].toString()}"
    runtime.string(result).toJSValue()
}

let EXPORT_MODULE1 = JSModule.registerFunc("testCJ02", hello)

// findUsage 6.2
let fn14: JSLambda = { context, callInfo =>
                return context.undefined().toJSValue()
            }
func fn_register14(runtime: JSContext): JSFunction {
    return runtime.function(fn14)
}
var tmp14 = JSModule.registerFunc("doSthThree14", fn_register14)

// findUsage 7.1
func setDataId(context: JSContext, callInfo: JSCallInfo) :JSValue {
    return callInfo.thisArg
}

func getDataId(context: JSContext, callInfo: JSCallInfo) : JSValue {
    return callInfo.thisArg
}

func ccc(runtime: JSContext): JSClass {
    let ctor: JSLambda = { context, callInfo =>
        return callInfo.thisArg
    }
    let clazz = runtime.clazz(ctor)
    clazz.addMethod(runtime.string("setId"), runtime.function(setDataId))
    clazz.addMethod(runtime.string("getId"), runtime.function(getDataId))
    clazz
}

let EXPORT_MODULE2 = JSModule.registerClass("SomeClass", ccc)

// findUsage 7.2
func setDataId15(runtime: JSContext, callInfo: JSCallInfo): JSValue {
 runtime.string("result").toJSValue()
}

func getDataId15(runtime: JSContext, callInfo: JSCallInfo): JSValue {
 runtime.string("result").toJSValue()
}

var tmp15 = JSModule.registerClass("SomeClass15") { context =>
    let ctor: JSLambda = { context, callInfo =>
        return callInfo.thisArg
    }
    let clazz = context.clazz(ctor)
    clazz.addMethod(context.string("setId15"), context.function(setDataId15))
    clazz.addMethod(context.string("getId15"), context.function(getDataId15))
    clazz
}
