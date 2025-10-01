// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
export declare namespace A {
    interface B {
    }
    
    class TestClass {
        name: string;
        getValue(): number;
    }
    
    enum TestEnum {
        VALUE1,
        VALUE2
    }
    
    class ClassWithMethod {
        method(a: number, b: string): boolean;
    }
    
    interface InterfaceWithMethod {
        prop: string;
        method(x: number): void;
    }
}
export declare namespace C.D {
    interface E {
    }
}
export declare namespace F {
    namespace G {
        interface H {
        }
    }
}

export declare namespace MathUtils {
    /**
     * Calculate the sum of two numbers
     */
    function add(a: number, b: number): number;
}

export declare namespace StringUtils {
    /**
     * Concatenate two strings (same signature as MathUtils.add)
     */
    function add(a: number, b: number): number;
}
