// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.collection.*
import std.ast.*
import std.regex.*
import stdx.serialization.serialization.*

public const i8: String = "Int8"
public const i16: String = "Int16"
public const i32: String = "Int32"
public const i64: String = "Int64"
public const u8: String = "UInt8"
public const u16: String = "UInt16"
public const u32: String = "UInt32"
public const u64: String = "UInt64"
public const f16: String = "Float16"
public const f32: String = "Float32"
public const f64: String = "Float64"
public const bool: String = "Bool"
public const str: String = "String"
public const unit: String = "Unit"
public const byteArr: String = "Array<UInt8>"
public const any: String = "Any"
public const promise: String = "Promise"
public const JsBigInt: String = "bigint"
public const JsHashMap: String = "Record"
public const AbilityContext: String = "common.Context"
public const UIContext: String = "common.UIAbilityContext"
public let syncKeyword: Array<String> = ["Callback", promise, "Async", "AsyncCallback"]
public let interopTypes: Array<String> = [i8, i16, i32, i64, u8, u16, u32, u64, f16, f32, f64, bool, str, unit, byteArr]
public const comma: String = ","
public let nl: Token = Token(TokenKind.NL)
public let i32Token: Token = Token(TokenKind.INT32)
public let i32opToken: Token = Token(TokenKind.IDENTIFIER, "?Int32")
public let strToken: Token = Token(TokenKind.IDENTIFIER, str)
public let stropToken: Token = Token(TokenKind.IDENTIFIER, "?String")
public let commaToken: Token = Token(TokenKind.COMMA)
public let noneToken: Token = Token(TokenKind.IDENTIFIER, "None")
public let deriveToStringToken: Token = Token(TokenKind.IDENTIFIER, "@Derive[ToString]")
public let cangjieKeyword = ["type", "in", "func", "enum", "package", "import", "class", "interface", "let", "var",
    "const", "init", "this", "super", "if", "else", "case", "try", "catch", "finally", "for", "do", "while", "throw",
    "return", "continue", "break", "is", "as", "match", "where", "extend", "spawn", "macro", "quote"]
public let ViewBase: Array<String> = ["AlphabetIndexer", "Blank", "Button", "Checkbox", "CheckboxGroup", "DataPanel",
    "Datepicker", "Divider", "Gauge", "Image", "ImageSpan", "LoadingProgress", "Menu", "MenuItem", "MenuItemGroup",
    "Progress", "QRCode", "Radio", "Rating", "RichEditor", "RichText", "ScrollBar", "Search", "Select", "Slider", "Span",
    "Stepper", "Stepperitem", "Text", "TextArea", "TextClock", "TextInput", "TextPicker", "TextTimer", "Toggle", "Video",
    "Web"]
public let ContainerBase: Array<String> = ["Badge", "Column", "ColumnSplit", "Flex", "GridCol", "GridRow", "Grid",
    "GridItem", "List", "ListItem", "ListItemGroup", "Panel", "Refresh", "RelativeContainer", "Row", "RowSplit",
    "Scroll", "Stack", "Swiper", "Tabs", "WaterFlow", "TabContent"]
public let ShapeComponent: Array<String> = ["Circle", "Line", "Path", "Rect", "Shape", "Polygon"]

// 记录用户自定义的类型名称（定义类、接口、枚举、泛型类型时更新）
public var userDefinedNames: HashSet<String> = HashSet<String>(interopTypes)

//记录一个namespace的内容
public class NameSpace {
    public var types: ArrayList<TypeType> = ArrayList<TypeType>()
    public var consts: ArrayList<VariableType> = ArrayList<VariableType>()
    public var enums: ArrayList<EnumType> = ArrayList<EnumType>()
    public var interfaces: HashMap<String, ObjectType> = HashMap<String, ObjectType>()
    public var classes: HashMap<String, ObjectType> = HashMap<String, ObjectType>()
    public var childrenMap: HashMap<String, ArrayList<String>> = HashMap<String, ArrayList<String>>()
    public var methods: ArrayList<MethodType> = ArrayList<MethodType>()
    public var spacePath: String = "" //记录namespace层级关系

    public func isEmpty(): Bool {
        return !(enums.size > 0 || interfaces.size > 0 || classes.size > 0 || methods.size > 0 || consts.size > 0 ||
            types.size > 0)
    }
}

//记录整个module的一些情况
public class Transform {
    public var useBigInt: Bool = false
    public var useContext: Bool = false
    public var useHashMap: Bool = false
    public var types: ArrayList<String> = ArrayList<String>()
    public var consts: ArrayList<String> = ArrayList<String>()
    public var interfaces: ArrayList<String> = ArrayList<String>()
    public var baseInterfaces: ArrayList<String> = ArrayList<String>()
    public var classes: ArrayList<String> = ArrayList<String>()
    public var baseClasses: ArrayList<String> = ArrayList<String>()
    public var enums: ArrayList<String> = ArrayList<String>()
    public var variables: ArrayList<String> = ArrayList<String>()
    public var namespaces: ArrayList<NameSpace> = ArrayList<NameSpace>()
    public var importInfo: ArrayList<String> = ArrayList<String>()
    public var symbols: HashMap<String, String> = HashMap<String, String>()
    public var typeParameters: ArrayList<TypeParameter> = ArrayList<TypeParameter>()

    public init() {}

    public init(fileContent: String) {
        useBigInt = fileContent.contains(JsBigInt)
        useContext = fileContent.contains(UIContext) || fileContent.contains("common.Context") || fileContent.contains(
            "Context")
        useHashMap = fileContent.contains(JsHashMap)
    }
}

// generic type for class or method
public class TypeParameter <: Serializable<TypeParameter> {
    public var name: String = ""

    public func serialize(): DataModel {
        var dms = DataModelStruct()
        dms = dms.add(field<String>("name", name))
        return dms
    }

    public static func deserialize(dm: DataModel): TypeParameter {
        let dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }

        let result = TypeParameter()
        result.name = String.deserialize(dms.get("name"))
        userDefinedNames.add(result.name)
        return result
    }
}

//描述一个ENUM
public class EnumType <: Serializable<EnumType> {
    protected var name: String = ""
    protected var keyValues: ArrayList<String> = ArrayList<String>()
    public var comment: String = ""

    public init() {
    }

    public init(_name: String, _keyValues: ArrayList<String>) {
        this.name = _name
        this.keyValues = _keyValues
        this.comment = ""
    }

    //暂时用不着
    public func serialize(): DataModel {
        var dms = DataModelStruct()
        dms = dms.add(field<String>("name", name))
        dms = dms.add(field<ArrayList<String>>("item-value", keyValues))
        dms = dms.add(field<String>("comment", comment))
        return dms
    }

    public static func deserialize(dm: DataModel): EnumType {
        let dms1 = match (dm) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }
        let dms2 = match (dms1.get("info")) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }
        let result = EnumType()
        result.name = String.deserialize(dms2.get("name"))
        result.keyValues = ArrayList<String>.deserialize(dms2.get("item-value"))
        result.comment = String.deserialize(dms2.get("comment"))
        userDefinedNames.add(result.name)
        return result
    }

    public func signature(): String {
        let n = keyValues.size
        var msg: String = ""
        msg = msg + " enum ${name} {\n"
        for (i in 0..n) {
            msg += "\t ${keyValues[i]}"
            if (i < n - 1) {
                msg += ",\n"
            } else {
                msg += "\n"
            }
        }
        msg = msg + "\t}"
        return msg
    }
}

//描述一个函数，可以是全局函数，也可以是成员函数
public class MethodType <: Serializable<MethodType> {
    public var namespace: ?NameSpace = None
    public var obj: ?ObjectType = None
    public var name: String = ""
    public var modifiers: ArrayList<String> = ArrayList<String>()
    public var params: ArrayList<String> = ArrayList<String>()
    let convertedParams: ArrayList<Parameter> = ArrayList<Parameter>()
    public var parameters: ArrayList<TsParameter> = ArrayList<TsParameter>()
    public var isConstructor: Bool = false
    public var returns: String = ""
    public var comment: String = ""
    public var typeParameters: ArrayList<TypeParameter> = ArrayList<TypeParameter>()

    public init() {}

    public func signature(): String {
        let n = params.size
        var msg: String = ""
        msg = msg + "${name}("
        for (i in 0..n) {
            msg += "${params[i]}"
            if (i < n - 1) {
                msg += comma + " "
            }
        }
        msg = msg + "): ${returns}"
        return msg
    }

    public func isStatic(): Bool {
        modifiers.contains("static")
    }

    //暂时用不着
    public func serialize(): DataModel {
        var dms = DataModelStruct()
        dms = dms.add(field<String>("name", name))
        dms = dms.add(field<ArrayList<String>>("param", params))
        dms = dms.add(field<String>("return", returns))
        dms = dms.add(field<ArrayList<String>>("modifier", modifiers))
        dms = dms.add(field<String>("comment", comment))
        dms = dms.add(field<Bool>("isConstructor", isConstructor))
        dms = dms.add(field<ArrayList<TypeParameter>>("typeParameters", typeParameters))
        return dms
    }

    public static func deserialize(dm: DataModel): MethodType {
        let dms1 = match (dm) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }
        let dms2 = match (dms1.get("info")) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }
        let result = MethodType()
        result.modifiers = ArrayList<String>.deserialize(dms2.get("modifier"))
        result.name = String.deserialize(dms2.get("name"))
        result.params = ArrayList<String>.deserialize(dms2.get("param"))
        result.returns = String.deserialize(dms2.get("return"))
        result.comment = String.deserialize(dms2.get("comment"))
        result.isConstructor = Bool.deserialize(dms2.get("isConstructor"))
        result.parameters = ArrayList<TsParameter>.deserialize(dms2.get("parameters"))
        result.typeParameters = ArrayList<TypeParameter>.deserialize(dms2.get("typeParameters"))
        return result
    }
}

//描述一个const常量
public class VariableType <: Serializable<VariableType> {
    public var name: String = ""
    public var value: String = ""
    public var varType: String = ""
    public var comment: String = ""

    public init() {}

    public init(_name: String, _value: String, _varType: String, _comment: String) {
        this.name = _name
        this.value = _value
        this.varType = _varType
        this.comment = _comment
    }

    public func signature(): String {
        "public const ${name} = ${value}"
    }

    //暂时用不着
    public func serialize(): DataModel {
        var dms = DataModelStruct()
        dms = dms.add(field<String>("name", name))
        dms = dms.add(field<String>("value", value))
        dms = dms.add(field<String>("type", varType))
        dms = dms.add(field<String>("comment", comment))
        return dms
    }

    public static func deserialize(dm: DataModel): VariableType {
        let dms1 = match (dm) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }
        let dms2 = match (dms1.get("info")) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }
        let result = VariableType()
        result.name = String.deserialize(dms2.get("name"))
        result.value = String.deserialize(dms2.get("value"))
        result.varType = String.deserialize(dms2.get("type"))
        result.comment = String.deserialize(dms2.get("comment"))
        return result
    }
}

//描述一个type alias
public class TypeType <: Serializable<TypeType> {
    public var name: String = ""
    public var comment: String = ""
    public var aliasType: String = ""
    public var tsType: TsType = TsAny

    public init() {}

    public init(_name: String, _comment: String, _type: String, _tsType: TsType) {
        this.name = _name
        this.comment = _comment
        this.aliasType = _type
        this.tsType = _tsType
    }

    public func signature(): String {
        "type ${name} = ${tsType}"
    }

    public func generate(): Tokens {
        let tok = Tokens()
        match (tsType) {
            case TsUnion(arr) =>
                // filt option type
                let nonNullTypes: ArrayList<TsType> = ArrayList<TsType>()
                for (e in arr) {
                    match (e) {
                        case TsNull | TsUndefined => ()
                        case _ => nonNullTypes.add(e)
                    }
                }
                if (nonNullTypes.size != 1) {
                    // handle union type, convert it to enum
                    let ut = UnionType(name, tsType)
                    tok.append(ut.genTokens())
                    return tok
                }
            case _ => ()
        }
        let nameToken = Token(TokenKind.IDENTIFIER, name)
        tok.append(quote(public type $(nameToken) = ))
        tok.append(Token(TokenKind.IDENTIFIER, tsType.toCJType()) + nl)
        return tok
    }

    //暂时用不着
    public func serialize(): DataModel {
        var dms = DataModelStruct()
        dms = dms.add(field<String>("name", name))
        dms = dms.add(field<String>("comment", comment))
        return dms
    }

    public static func deserialize(dm: DataModel): TypeType {
        let dms1 = match (dm) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }
        let dms2 = match (dms1.get("info")) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }
        let result = TypeType(String.deserialize(dms2.get("name")), String.deserialize(dms2.get("comment")),
            String.deserialize(dms2.get("type")), TsType.fromSExp(String.deserialize(dms2.get("typeNode"))))
        userDefinedNames.add(result.name)
        return result
    }
}

//描述一个对象的成员变量
public class Property <: Serializable<Property> & Comparable<Property> {
    public var propKey: String = ""
    public var propValue: String = ""
    public var propType: String = ""
    public var modifiers: ArrayList<String> = ArrayList<String>()
    public var comment: String = ""
    public var isOptional: Bool = false
    public var defValue: ?String = None
    public var cjKey: String = ""
    public var cjType: String = ""
    public var isCallbackType: Bool = false
    public var callbackType: CallbackType = CallbackType()
    public var typeNode: TsType = TsAny
    public var inferredType: TsType = TsAny

    public init() {}

    public func compare(that: Property): Ordering {
        match ((this.isOptional, that.isOptional)) {
            case (true, false) => GT
            case (false, true) => LT
            case _ => EQ
        }
    }

    public func signature(): String {
        var msg: String = ""
        var optionFlg = ""
        if (isOptional) {
            optionFlg = "?"
        }
        for (i in 0..modifiers.size) {
            msg += modifiers[i]
        }
        if (propValue != "") {
            msg += "${propKey}${optionFlg}: ${propType} = ${propValue};"
        } else {
            msg += "${propKey}${optionFlg}: ${propType};"
        }
        return msg
    }

    public func isReadOnly(): Bool {
        modifiers.contains("ReadonlyKeyword")
    }

    public func isStatic(): Bool {
        modifiers.contains("StaticKeyword")
    }

    public func serialize(): DataModel {
        var dms = DataModelStruct()
        dms = dms.add(field<String>("prop_key", propKey))
        dms = dms.add(field<String>("prop_value", propValue))
        dms = dms.add(field<String>("prop_type", propType))
        dms = dms.add(field<ArrayList<String>>("modifiers", modifiers))
        dms = dms.add(field<String>("comment", comment))
        return dms
    }

    public static func deserialize(dm: DataModel): Property {
        let dms1 = match (dm) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }
        let dms2 = match (dms1.get("info")) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }
        let result = Property()
        result.propKey = String.deserialize(dms2.get("name"))
        result.propValue = String.deserialize(dms2.get("value"))
        result.propType = String.deserialize(dms2.get("type"))
        result.modifiers = ArrayList<String>.deserialize(dms2.get("modifier"))
        result.comment = String.deserialize(dms2.get("comment"))
        result.isOptional = Bool.deserialize(dms2.get("isOptional"))
        if (result.comment.contains("@default")) {
            let idx = result.comment.indexOf("@default")
            if (let Some(i) <- idx) {
                let tmp = result.comment[i..].split("\n")[0]
                result.defValue = tmp.split(" ", 2)[1].replace("\r", "").replace("\n", "")
            }
        }
        result.cjKey = checkCangjieKeyword(result.propKey)
        result.typeNode = TsType.fromSExp(String.deserialize(dms2.get("typeNode")))
        result.cjType = result.typeNode.toCJType()
        return result
    }
}

//描述一个对象，可以是arkts的interface/class，也可以是仓颉的class/struct
public class ObjectType <: Serializable<ObjectType> & Equatable<ObjectType> {
    public var name: String = ""
    public var modifiers: ArrayList<String> = ArrayList<String>()
    public var parents: ArrayList<String> = ArrayList<String>()
    public var parentsProperties: ArrayList<Property> = ArrayList<Property>()
    public var properties: ArrayList<Property> = ArrayList<Property>()
    public var parentsMethods: ArrayList<MethodType> = ArrayList<MethodType>()
    public var methods: ArrayList<MethodType> = ArrayList<MethodType>()
    public var comment: String = ""
    public var typeParameters: ArrayList<TypeParameter> = ArrayList<TypeParameter>()
    public var nameSpace: NameSpace = NameSpace()

    public init() {}

    public operator func ==(that: ObjectType): Bool {
        return this.name == that.name && this.modifiers == that.modifiers && this.parents == that.parents &&
            this.comment == that.comment
    }

    public operator func !=(that: ObjectType) {
        return !(this == that)
    }

    public func signature(): String {
        var msg: String = ""

        for (i in 0..modifiers.size) {
            msg += " ${modifiers[i]}"
        }

        msg = msg + " ${name} {\n"
        for (i in 0..properties.size) {
            msg += "    ${properties[i].signature()}\n"
        }
        for (i in 0..methods.size) {
            msg += "    ${methods[i].signature()}\n"
        }
        msg = msg + "    }"
        return msg
    }

    public func isChild(): Bool {
        parents.size > 0
    }

    public func isMI(): Bool {
        parents.size > 1
    }

    public func serialize(): DataModel {
        var dms = DataModelStruct()
        dms = dms.add(field<String>("name", name))
        dms = dms.add(field<ArrayList<String>>("modifiers", modifiers))
        dms = dms.add(field<ArrayList<String>>("parents", parents))
        dms = dms.add(field<ArrayList<TypeParameter>>("typeParameters", typeParameters))
        dms = dms.add(field<ArrayList<Property>>("properties", properties))
        dms = dms.add(field<ArrayList<MethodType>>("methods", methods))
        dms = dms.add(field<String>("comment", comment))
        return dms
    }

    public static func deserialize(dm: DataModel): ObjectType {
        let dms1 = match (dm) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }
        let dms2 = match (dms1.get("info")) {
            case data: DataModelStruct => data
            case _ => throw IllegalArgumentException("this data is not DataModelStruct")
        }

        let result = ObjectType()
        result.name = String.deserialize(dms2.get("name"))
        result.modifiers = ArrayList<String>.deserialize(dms2.get("modifier"))
        result.parents = ArrayList<String>.deserialize(dms2.get("father"))
        result.typeParameters = ArrayList<TypeParameter>.deserialize(dms2.get("typeParameters"))
        result.properties = ArrayList<Property>.deserialize(dms1.get("variable"))
        result.methods = ArrayList<MethodType>.deserialize(dms1.get("method"))
        for (m in result.methods) {
            m.obj = result
        }
        result.comment = String.deserialize(dms2.get("comment"))
        userDefinedNames.add(result.name)
        return result
    }
}

//描述一个联合类型
public class UnionType {
    let name: String
    // metaInfo like: string | number
    let metaInfo: TsType
    let typeList = ArrayList<String>()
    let ctxName = "context"
    let nativeTypeMap = HashMap<String, (String, String, String)>(
        [
            ("null", ("NULL", "NULL", "${ctxName}.null().toJSValue()")),
            ("undefined", ("UNDEFINED", "UNDEFINED", "${ctxName}.undefined().toJSValue()")),
            ("number", ("NUMBER(Float64)", "NUMBER(x)", "${ctxName}.number(x).toJSValue()")),
            ("string", ("STRING(String)", "STRING(x)", "${ctxName}.string(x).toJSValue()")),
            ("boolean", ("BOOLEAN(Bool)", "BOOLEAN(x)", "${ctxName}.boolean(x).toJSValue()")),
            ("Uint8Array", ("BYTEARRAY(Array<UInt8>)", "BYTEARRAY(x)",
                    "${ctxName}.global[\"Uint8Array\"].asClass().new(x.toJSValue(${ctxName}))")),
            ("Float32Array", ("FLOAT32ARRAY(Array<Float32>)", "FLOAT32ARRAY(x)",
                    "${ctxName}.global[\"Float32Array\"].asClass().new(${ctxName}.arrayBuffer(x).toJSValue())")),
            ("Int8Array", ("INT8ARRAY(Array<Int8>)", "INT8ARRAY(x)",
                    "${ctxName}.global[\"Int8Array\"].asClass().new(${ctxName}.arrayBuffer(x).toJSValue())")),
            ("Int16Array", ("INT16ARRAY(Array<Int16>)", "INT16ARRAY(x)",
                    "${ctxName}.global[\"Int16Array\"].asClass().new(${ctxName}.arrayBuffer(x).toJSValue())")),
            ("Uint16Array", ("UINT16ARRAY(Array<UInt16>)", "UINT16ARRAY(x)",
                    "${ctxName}.global[\"Uint16Array\"].asClass().new(${ctxName}.arrayBuffer(x).toJSValue())")),
            ("Uint32Array", ("UINT32ARRAY(Array<UInt32>)", "UINT32ARRAY(x)",
                    "${ctxName}.global[\"Uint32Array\"].asClass().new(${ctxName}.arrayBuffer(x).toJSValue())")),
            ("Int32Array", ("INT32ARRAY(Array<Int32>)", "INT32ARRAY(x)",
                    "${ctxName}.global[\"Int32Array\"].asClass().new(${ctxName}.arrayBuffer(x).toJSValue())")),
            ("BigInt64Array", ("INT64ARRAY(Array<Int64>)", "INT64ARRAY(x)",
                    "${ctxName}.global[\"BigInt64Array\"].asClass().new(${ctxName}.arrayBuffer(x).toJSValue())")),
            ("BigUint64Array", ("UINT64ARRAY(Array<UInt64>)", "UINT64ARRAY(x)",
                    "${ctxName}.global[\"BigUint64Array\"].asClass().new(${ctxName}.arrayBuffer(x).toJSValue())")),
            ("Float64Array", ("FLOAT64ARRAY(Array<Float64>)", "FLOAT64ARRAY(x)",
                    "${ctxName}.global[\"Float64Array\"].asClass().new(${ctxName}.arrayBuffer(x).toJSValue())")),
            ("bigint", ("BIGINT(BigInt)", "BIGINT(x)", "${ctxName}.bigint(x).toJSValue()"))
        ]
    )

    public init(_name: String, _metaInfo: TsType) {
        name = _name
        metaInfo = _metaInfo
        match (metaInfo) {
            case TsUnion(list) => typeList.add(all: list.map {it => it.toString()})
            case _ => throw IllegalArgumentException("Input is not a union type")
        }
    }

    private func isStringLiteral(t: String): Bool {
        (t.startsWith("\"") && t.endsWith("\"")) || (t.startsWith("\'") && t.endsWith("\'"))
    }

    private func genIdentifier(t: String): String {
        return Regex("[^0-9a-zA-Z_]").replaceAll(t.replace(" ", ""), "_").toAsciiUpper()
    }

    public func signature(): String {
        var ret: String = ""

        ret = ret + "public enum ${name} {\n"

        for (t in typeList) {
            let s = getStringToken(t)
            ret = ret + ("\t | ${s} \n")
        }

        ret = ret + "\t}\n"
        ret
    }

    func getStringToken(t: String): String {
        return if (let Some(v) <- nativeTypeMap.get(t)) {
            // handle native type
            v[0]
        } else if (userDefinedNames.contains(t)) {
            // handle user-defined type
            t.toAsciiUpper() + "(${t})"
        } else if (isStringLiteral(t)) {
            // String Literal
            t.trimStart("\"").trimStart("\'").trimEnd("\"").trimEnd("\'").toAsciiUpper()
        } else {
            // handle unknown type
            genIdentifier(t) + "(JSValue/* FIXME: `${t}` */)"
        }
    }

    func getIdentifierToken(t: String): (Token, Token) {
        return if (let Some(c) <- nativeTypeMap.get(t)) {
            (Token(TokenKind.IDENTIFIER, c[1]), Token(TokenKind.IDENTIFIER, c[2]))
        } else if (userDefinedNames.contains(t)) {
            let x = Token(TokenKind.IDENTIFIER, t.toAsciiUpper() + "(x)")
            let y = Token(TokenKind.IDENTIFIER, "x.toJSValue(context)")
            (x, y)
        } else if (isStringLiteral(t)) {
            let tmp = t.trimStart("\"").trimStart("\'").trimEnd("\"").trimEnd("\'")
            let x = Token(TokenKind.IDENTIFIER, tmp.toAsciiUpper())
            let y = Token(TokenKind.IDENTIFIER, "${ctxName}.string(\"${tmp}\").toJSValue()")
            (x, y)
        } else {
            let x = Token(TokenKind.IDENTIFIER, genIdentifier(t) + "(x)")
            let y = Token(TokenKind.IDENTIFIER, "x")
            (x, y)
        }
    }

    public func genTokens(): Tokens {
        var ret = Tokens()
        let nameToken = Token(TokenKind.IDENTIFIER, name)
        ret.append(quote(public enum $(nameToken) { ) + nl)
        for (t in typeList) {
            let vToken = Token(TokenKind.IDENTIFIER, getStringToken(t))
            ret.append(quote(| $(vToken)) + nl)
        }
        ret.append(nl)

        let ctxToken = Token(TokenKind.IDENTIFIER, ctxName)
        ret.append(quote(public func toJSValue($(ctxToken): JSContext): JSValue {) + nl)

        // toJSValue function body
        ret.append(quote(match(this) {) + nl)
        for (t in typeList) {
            let (cs, cv) = getIdentifierToken(t)
            ret.append(quote(case $(cs) => ) + cv + nl)
        }

        ret.append(quote(}) + nl + quote(}) + nl + quote(}) + nl)
        ret
    }
}

struct Parameter {
    let paramName: String
    let paramType: String // todo: Consider nested callbacks. This should be a callback type.
    let isOptional: Bool

    init(_name: String, _type: String, _isOptional: Bool) {
        paramName = _name
        var t = TsType.fromSExp(wrapTypeForParser(_name))
        paramType = t.toCJType()
        isOptional = _isOptional
    }

    // Automatically analyse forms like `name?: type`.
    init(str: String) {
        let splits = str.split(":", 2)
        if (splits[0].endsWith("?")) {
            paramName = splits[0][..splits[0].size - 1]
            isOptional = true
        } else {
            paramName = splits[0]
            isOptional = false
        }
        var t = TsType.fromSExp(wrapTypeForParser(splits[1]))

        paramType = t.toCJType()
    }

    func print(inMethod!: ?Bool = false): String {
        var symbol = ": "
        if (let Some(v) <- inMethod) {
            if (v) {
                symbol = "!" + symbol
            }
        }
        return paramName + symbol + if (isOptional) {
            "?"
        } else {
            ""
        } + paramType
    }
}

//描述一个回调类型
public class CallbackType {
    let params: ArrayList<Parameter> = ArrayList<Parameter>()
    let returnType: String

    public init() {
        returnType = ""
    }

    public init(str: String) {
        /* Analyse forms like
            `Callback<void>`
            `Callback<type>`
            `Callback<type, returnType>`
         */
        if (str.startsWith("Callback")) {
            let param = str["Callback<".size..str.size - 1].replace(" ", "")
            // If no return type, let `splits[1]` convert to `unit`.
            let splits = (param + ",void").split(",")
            // `type == void` means no parameters.
            if (splits[0] != "void") {
                params.add(Parameter("parameter", splits[0], false))
            }
            var t = TsType.fromSExp(wrapTypeForParser(splits[1]))

            returnType = t.toCJType()
            return
        }
            // Analyse forms like `(name: type, name1?: type1) => returnType`.
            else {
            let splits = str.replace(" ", "").split("=>")
            for (param in splits[0][1..splits[0].size - 1].split(",")) {
                params.add(Parameter(param))
            }
            var t = TsType.fromSExp(wrapTypeForParser(splits[1]))

            returnType = t.toCJType()
        }
    }

    public func print(): String {
        return "(" + (params |> map {p => p.print()} |> collectString<String>(delimiter: ", ")) + ") -> " + returnType
    }

    public func printParams(): String {
        return params |> map {p => p.print()} |> collectString<String>(delimiter: ", ")
    }
}

public class CommandLineArg {
    public var arkpath: String = "" //  输入的ArkTS文件路径
    public var outDir: String = "" //  输出的Cangjie文件路径
    public var jsASTPath: String = "" //  ArkTS文件分析器路径
    public var cjModuleName: String = ""
    public var is3rd: Bool = true
    public var directoryPath: String = ""
    public var tsPath: String = ""
}

// Helper function to wrap type names appropriately for TypeParser
func wrapTypeForParser(typeName: String): String {
    let basicTypes = ["number", "string", "boolean", "bigint", "symbol", "void", "never", "undefined", "null", "any"]
    
    // If the type already starts with '(', check if it's already an S-expression
    if (typeName.startsWith("(")) {
        // Check if it's already a proper S-expression (starts with known tags)
        let knownTags = ["function", "ref", "union", "intersection", "tuple", "array", "optional", "literal", "unsupported"]
        let firstToken = typeName[1..].split(" ")[0]
        if (knownTags.contains(firstToken)) {
            return typeName
        }
        
        // If it's TypeScript function syntax like "(s: String) => void", return as unsupported for now
        if (typeName.contains("=>")) {
            return "(unsupported function)"
        }
        
        // Otherwise, assume it's already an S-expression
        return typeName
    }
    
    // Handle Promise types specifically
    if (typeName.startsWith("Promise<") && typeName.endsWith(">")) {
        let innerType = typeName[8..(typeName.size - 1)] // Remove "Promise<" and ">"
        let innerTypeSExp = wrapTypeForParser(innerType)
        return "(ref Promise ${innerTypeSExp})"
    }
    
    // Handle inline object literal types like { a: number } or {}
    // No String.trim() in std; emulate by removing common whitespace
    let _t = typeName.replace(" ", "").replace("\t", "").replace("\n", "").replace("\r", "")
    if (_t.startsWith("{")) {
        return "(object)"
    }

    // Handle array types (e.g., number[], string[])
    if (typeName.endsWith("[]")) {
        let elementType = typeName[0..(typeName.size - 2)]
        let elementTypeSExp = wrapTypeForParser(elementType)
        return "(array ${elementTypeSExp})"
    }
    
    // Handle typeof queries early: typeof X -> treat as X
    if (typeName.startsWith("typeof ")) {
        let target = typeName["typeof ".size..]
            .replace(" ", "").replace("\t", "").replace("\n", "").replace("\r", "")
        return wrapTypeForParser(target)
    }

    // Handle function types (s: String) => void - check before generic types
    if (typeName.contains("=>")) {
        return convertFunctionToSExp(typeName)
    }
    
    // Handle union types (e.g., string | number, string | Promise<string>)
    if (typeName.contains("|")) {
        return convertUnionToSExp(typeName)
    }
    
    // If it contains angle brackets, it's a generic type - return as unsupported for now
    if (typeName.contains("<") || typeName.contains(">")) {
        return "(unsupported generic)"
    }
    
    // Handle intersection types before special-character fallback
    if (typeName.contains("&")) {
        return convertIntersectionToSExp(typeName)
    }
    // If it contains forward slashes or other special characters, return as unsupported for now
    if (typeName.contains("/") || typeName.contains("\\")) {
        return "(unsupported special)"
    }
    
    // If it's a basic type, wrap with parentheses
    if (basicTypes.contains(typeName)) {
        return "(${typeName})"
    } else {
        // For user-defined types, wrap with ref
        return "(ref ${typeName})"
    }
}

// Convert function type to S-expression (matching old TypeMap behavior)
func convertFunctionToSExp(functionType: String): String {
    println("DEBUG: convertFunctionToSExp called with: ${functionType}")
    // Remove outer parentheses if present: ((s: String) => void) -> (s: String) => void
    let tmp = functionType.replace("\r", "").replace("\n", "").replace("(", "").replace(")", "")
    println("DEBUG: After removing parentheses: ${tmp}")
    
    // Split by => to get parameters and return type
    let parts = tmp.split("=>")
    println("DEBUG: Split parts: ${parts}")
    if (parts.size != 2) {
        println("DEBUG: Invalid function type format")
        return "(unsupported function)"
    }
    
    let paramPart = parts[0].replace(" ", "").replace("\t", "").replace("\n", "").replace("\r", "")
    let returnType = parts[1].replace(" ", "").replace("\t", "").replace("\n", "").replace("\r", "")
    
    // Convert return type using old TypeMap logic
    let returnTypeCJ = convertTypeToCJ(returnType)
    
    if (paramPart.isEmpty()) {
        // No parameters: () => void
        return "(function ${returnTypeCJ} ())"
    } else {
        // Parse parameters using old TypeMap logic
        let params = parseFunctionParamsOldStyle(paramPart)
        return "(function ${returnTypeCJ} ${params})"
    }
}

// Convert type to Cangjie type (matching old TypeMap behavior)
func convertTypeToCJ(typeName: String): String {
    match (typeName) {
        case "string" => "str"
        case "number" => "f64"
        case "void" => "unit"
        case "boolean" => "bool"
        case "bigint" => "BigInt"
        case "any" => "any"
        case _ => typeName
    }
}

// Parse function parameters (matching old TypeMap behavior)
func parseFunctionParamsOldStyle(paramStr: String): String {
    let params = parseGenericParams(paramStr)
    var result = ""
    for (i in 0..params.size) {
        let param = params[i]
        if (param.isEmpty()) {
            continue
        }
        let split = param.split(":")
        if (split.size == 2) {
            let paramName = split[0].replace("?", "")
            let paramType = split[1]
            let paramTypeCJ = convertTypeToCJ(paramType)
            result = result + " (${paramName} ${paramTypeCJ})"
        }
    }
    return result
}

// Parse function parameters: "s: String, n: number" -> "(s String) (n number)"
func parseFunctionParams(paramStr: String): String {
    let params = parseGenericParams(paramStr)
    var result = ""
    for (i in 0..params.size) {
        let param = params[i]
        if (param.isEmpty()) {
            continue
        }
        let split = param.split(":")
        if (split.size == 2) {
            let paramName = split[0].replace("?", "")
            let paramType = split[1]
            let paramTypeSExp = wrapTypeForParser(paramType)
            result = result + " (${paramName} ${paramTypeSExp})"
        }
    }
    return result
}

// Convert union type to S-expression
func convertUnionToSExp(unionType: String): String {
    let arr = unionType.split("|")
    let nonNullTypes: ArrayList<String> = ArrayList<String>()
    
    for (e in arr) {
        let trimmed = e.replace(" ", "").replace("\t", "").replace("\n", "").replace("\r", "")
        if (trimmed != "null" && trimmed != "undefined") {
            nonNullTypes.add(trimmed)
        }
    }
    
    if (nonNullTypes.size == 1) {
        // T|null or T|undefined -> Option<T>
        let innerTypeSExp = wrapTypeForParser(nonNullTypes[0])
        return "(ref Option ${innerTypeSExp})"
    } else if (nonNullTypes.size == 2) {
        // T1|T2 -> Union of two types
        let type1SExp = wrapTypeForParser(nonNullTypes[0])
        let type2SExp = wrapTypeForParser(nonNullTypes[1])
        return "(union ${type1SExp} ${type2SExp})"
    } else {
        // More than 2 types - return as unsupported for now
        return "(unsupported union)"
    }
}

// Convert intersection type to S-expression
func convertIntersectionToSExp(intersectionType: String): String {
    let arr = intersectionType.split("&")
    let parts: ArrayList<String> = ArrayList<String>()

    for (e in arr) {
        let trimmed = e.replace(" ", "").replace("\t", "").replace("\n", "").replace("\r", "")
        if (!trimmed.isEmpty()) {
            parts.add(trimmed)
        }
    }

    if (parts.size == 0) {
        return "(unsupported intersection)"
    } else if (parts.size == 1) {
        return wrapTypeForParser(parts[0])
    } else if (parts.size == 2) {
        let t1 = wrapTypeForParser(parts[0])
        let t2 = wrapTypeForParser(parts[1])
        return "(intersection ${t1} ${t2})"
    } else {
        // Reduce N-way intersection to nested binary intersections
        var expr = wrapTypeForParser(parts[0])
        for (i in 1..parts.size) {
            let next = wrapTypeForParser(parts[i])
            expr = "(intersection ${expr} ${next})"
        }
        return expr
    }
}

// Parse generic parameters (similar to handleGeneric in old TypeMap)
func parseGenericParams(lambda: String): Array<String> {
    if (let None <- lambda.indexOf("<")) {
        return lambda.split(",")
    }
    
    let byteArr = lambda.toArray()
    var leftBracketCount = 0
    for (i in 0..byteArr.size) {
        if (byteArr[i] == 60) { // '<' character
            leftBracketCount = leftBracketCount + 1
        } else if (byteArr[i] == 62) { // '>' character
            leftBracketCount = leftBracketCount - 1
        } else if (byteArr[i] == 44 && leftBracketCount == 0) { // ',' character
            byteArr[i] = 10 // '\n' character
        }
    }
    
    let newStr = String.fromUtf8(byteArr)
    let split = newStr.split("\n")
    for (i in 0..split.size) {
        split[i] = split[i].replace("\n", ",")
    }
    split
}
