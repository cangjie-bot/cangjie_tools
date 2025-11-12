// cjbind 0.2.7 生成，请勿编辑, DO NOT EDIT

package cjbind_ffi

public const INT_MACRO: Int64 = 42i64
public const FLOAT_MACRO: Float64 = 3.140000104904175
public const CHAR_MACRO: Int64 = 65i64
public const E1_A: E1 = 0
public const E1_B: E1 = 1
public const E1_C: E1 = 2

public type E1 = UInt32

public const UA: _cjbind_ty_1 = 0
public const UB: _cjbind_ty_1 = 1
public const UC: _cjbind_ty_1 = 2

public type _cjbind_ty_1 = UInt32

@C
public struct Some_ {
    public let a: Int32
    public let b: UInt8

    public init(a: Int32, b: UInt8) {
        this.a = a
        this.b = b
    }
}

@When[debug]
let _ = {
    =>
        let size = sizeOf<Some_>()
        if (size != 8) {
            throw Exception("Size of Some is not 8 but ${size} bytes")
        }
        let align = alignOf<Some_>()
        if (align != 4) {
            throw Exception("Alignment of Some is not 4 but ${align} bytes")
        }
}

foreign func f(arg1: CPointer<UInt8>): Unit

// String value: hello
public const ss: VArray<UInt8, $6> = [104, 101, 108, 108, 111, 0]
