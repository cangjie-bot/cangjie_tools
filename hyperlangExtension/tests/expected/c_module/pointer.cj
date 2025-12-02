// cjbind 0.2.7 生成，请勿编辑, DO NOT EDIT

package my_module

public const num: Int32 = 42i64

public type callback_func = CFunc<(arg1: Int32, arg2: Int32) -> Int32>

foreign func testPointer(a: Int32): CPointer<Unit>
