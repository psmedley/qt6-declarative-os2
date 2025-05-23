/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QV4COMPILEDDATA_P_H
#define QV4COMPILEDDATA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <functional>

#include <QtCore/qhashfunctions.h>
#include <QtCore/qstring.h>
#include <QtCore/qscopeguard.h>
#include <QtCore/qvector.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qhash.h>
#include <QtCore/qversionnumber.h>
#include <QtCore/qlocale.h>

#if QT_CONFIG(temporaryfile)
#include <QtCore/qsavefile.h>
#endif

#include <private/qendian_p.h>
#include <private/qv4staticvalue_p.h>
#include <functional>
#include <limits.h>

QT_BEGIN_NAMESPACE

// Bump this whenever the compiler data structures change in an incompatible way.
//
// IMPORTANT:
//
// Also change the comment behind the number to describe the latest change. This has the added
// benefit that if another patch changes the version too, it will result in a merge conflict, and
// not get removed silently.

// Note: We got two different versions 0x36 now, one with builtin value types and one without.
//       However, the one without is exclusive to Qt 6.4+ and the compiled data cannot be preserved
//       across Qt versions. Therefore, this is fine.
#define QV4_DATA_STRUCTURE_VERSION 0x36 // reordered runtime functions when compiling at run time

class QIODevice;
class QQmlTypeNameCache;
class QQmlType;
class QQmlEngine;

namespace QQmlPrivate {
struct AOTCompiledFunction;
}

namespace QmlIR {
struct Document;
}

namespace QV4 {
namespace Heap {
struct Module;
struct String;
struct InternalClass;
};

struct Function;
class EvalISelFactory;

namespace CompiledData {

struct String;
struct Function;
struct Lookup;
struct RegExp;
struct Unit;

template <typename ItemType, typename Container, const ItemType *(Container::*IndexedGetter)(int index) const>
struct TableIterator
{
    TableIterator(const Container *container, int index) : container(container), index(index) {}
    const Container *container;
    int index;

    const ItemType *operator->() { return (container->*IndexedGetter)(index); }
    ItemType operator*() {return *operator->();}
    void operator++() { ++index; }
    bool operator==(const TableIterator &rhs) const { return index == rhs.index; }
    bool operator!=(const TableIterator &rhs) const { return index != rhs.index; }
};

struct Location
{
    Location() : m_data(QSpecialIntegerBitfieldZero) {}
    Location(quint32 l, quint32 c) : Location()
    {
        m_data.set<LineField>(l);
        m_data.set<ColumnField>(c);
        Q_ASSERT(m_data.get<LineField>() == l);
        Q_ASSERT(m_data.get<ColumnField>() == c);
    }

    inline bool operator<(const Location &other) const {
        return m_data.get<LineField>() < other.m_data.get<LineField>()
                || (m_data.get<LineField>() == other.m_data.get<LineField>()
                    && m_data.get<ColumnField>() < other.m_data.get<ColumnField>());
    }

    friend size_t qHash(const Location &location, size_t seed = 0)
    {
        return QT_PREPEND_NAMESPACE(qHash)(location.m_data.data(), seed);
    }

    friend bool operator==(const Location &a, const Location &b)
    {
        return a.m_data.data()== b.m_data.data();
    }

    void set(quint32 line, quint32 column)
    {
        m_data.set<LineField>(line);
        m_data.set<ColumnField>(column);
    }

    quint32 line() const { return m_data.get<LineField>(); }
    quint32 column() const { return m_data.get<ColumnField>(); }

private:
    using LineField = quint32_le_bitfield_member<0, 20>;
    using ColumnField = quint32_le_bitfield_member<20, 12>;

    quint32_le_bitfield_union<LineField, ColumnField> m_data;
};
static_assert(sizeof(Location) == 4, "Location structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct RegExp
{
    enum Flags : unsigned int {
        RegExp_NoFlags    = 0x0,
        RegExp_Global     = 0x01,
        RegExp_IgnoreCase = 0x02,
        RegExp_Multiline  = 0x04,
        RegExp_Unicode    = 0x08,
        RegExp_Sticky     = 0x10
    };

    RegExp() : m_data(QSpecialIntegerBitfieldZero) {}
    RegExp(quint32 flags, quint32 stringIndex) : RegExp()
    {
        m_data.set<FlagsField>(flags);
        m_data.set<StringIndexField>(stringIndex);
    }

    quint32 flags() const { return m_data.get<FlagsField>(); }
    quint32 stringIndex() const { return m_data.get<StringIndexField>(); }

private:
    using FlagsField = quint32_le_bitfield_member<0, 5>;
    using StringIndexField = quint32_le_bitfield_member<5, 27>;
    quint32_le_bitfield_union<FlagsField, StringIndexField> m_data;
};
static_assert(sizeof(RegExp) == 4, "RegExp structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct Lookup
{
    enum Type : unsigned int {
        Type_Getter = 0,
        Type_Setter = 1,
        Type_GlobalGetter = 2,
        Type_QmlContextPropertyGetter = 3
    };

    quint32 typeAndFlags() const { return m_data.get<TypeAndFlagsField>(); }
    quint32 nameIndex() const { return m_data.get<NameIndexField>(); }

    Lookup() : m_data(QSpecialIntegerBitfieldZero) {}
    Lookup(Type type, quint32 nameIndex) : Lookup()
    {
        m_data.set<TypeAndFlagsField>(type);
        m_data.set<NameIndexField>(nameIndex);
    }

private:
    using TypeAndFlagsField = quint32_le_bitfield_member<0, 4>;
    using NameIndexField = quint32_le_bitfield_member<4, 28>;
    quint32_le_bitfield_union<TypeAndFlagsField, NameIndexField> m_data;
};
static_assert(sizeof(Lookup) == 4, "Lookup structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct JSClassMember
{
    JSClassMember() : m_data(QSpecialIntegerBitfieldZero) {}

    void set(quint32 nameOffset, bool isAccessor)
    {
        m_data.set<NameOffsetField>(nameOffset);
        m_data.set<IsAccessorField>(isAccessor ? 1 : 0);
    }

    quint32 nameOffset() const { return m_data.get<NameOffsetField>(); }
    bool isAccessor() const { return m_data.get<IsAccessorField>() != 0; }

private:
    using NameOffsetField = quint32_le_bitfield_member<0, 31>;
    using IsAccessorField = quint32_le_bitfield_member<31, 1>;
    quint32_le_bitfield_union<NameOffsetField, IsAccessorField> m_data;
};
static_assert(sizeof(JSClassMember) == 4, "JSClassMember structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct JSClass
{
    quint32_le nMembers;
    // JSClassMember[nMembers]

    static int calculateSize(int nMembers) { return (sizeof(JSClass) + nMembers * sizeof(JSClassMember) + 7) & ~7; }
};
static_assert(sizeof(JSClass) == 4, "JSClass structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct String
{
    qint32_le size;

    static int calculateSize(const QString &str) {
        return (sizeof(String) + (str.length() + 1) * sizeof(quint16) + 7) & ~0x7;
    }
};

static_assert (sizeof (String) == 4, "String structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct CodeOffsetToLine {
    quint32_le codeOffset;
    quint32_le line;
};
static_assert(sizeof(CodeOffsetToLine) == 8, "CodeOffsetToLine structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct Block
{
    quint32_le nLocals;
    quint32_le localsOffset;
    quint16_le sizeOfLocalTemporalDeadZone;
    quint16_le padding;

    const quint32_le *localsTable() const { return reinterpret_cast<const quint32_le *>(reinterpret_cast<const char *>(this) + localsOffset); }

    static int calculateSize(int nLocals) {
        int trailingData = nLocals*sizeof (quint32);
        size_t size = align(align(sizeof(Block)) + size_t(trailingData));
        Q_ASSERT(size < INT_MAX);
        return int(size);
    }

    static size_t align(size_t a) {
        return (a + 7) & ~size_t(7);
    }
};
static_assert(sizeof(Block) == 12, "Block structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

enum class BuiltinType : unsigned int {
    Var = 0, Int, Bool, Real, String, Url, Color,
    Font, Time, Date, DateTime, Rect, Point, Size,
    Vector2D, Vector3D, Vector4D, Matrix4x4, Quaternion, InvalidBuiltin
};

struct ParameterType
{
    void set(bool indexIsBuiltinType, quint32 typeNameIndexOrBuiltinType)
    {
        m_data.set<IndexIsBuiltinTypeField>(indexIsBuiltinType ? 1 : 0);
        m_data.set<TypeNameIndexOrBuiltinTypeField>(typeNameIndexOrBuiltinType);
    }

    bool indexIsBuiltinType() const
    {
        return m_data.get<IndexIsBuiltinTypeField>() != 0;
    }

    quint32 typeNameIndexOrBuiltinType() const
    {
        return m_data.get<TypeNameIndexOrBuiltinTypeField>();
    }

private:
    using IndexIsBuiltinTypeField = quint32_le_bitfield_member<0, 1>;
    using TypeNameIndexOrBuiltinTypeField = quint32_le_bitfield_member<1, 31>;
    quint32_le_bitfield_union<IndexIsBuiltinTypeField, TypeNameIndexOrBuiltinTypeField> m_data;
};
static_assert(sizeof(ParameterType) == 4, "ParameterType structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct Parameter
{
    quint32_le nameIndex;
    ParameterType type;
};
static_assert(sizeof(Parameter) == 8, "Parameter structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

// Function is aligned on an 8-byte boundary to make sure there are no bus errors or penalties
// for unaligned access. The ordering of the fields is also from largest to smallest.
struct Function
{
    enum Flags : unsigned int {
        IsStrict            = 0x1,
        IsArrowFunction     = 0x2,
        IsGenerator         = 0x4,
        IsClosureWrapper    = 0x8,
    };

    // Absolute offset into file where the code for this function is located.
    quint32_le codeOffset;
    quint32_le codeSize;

    quint32_le nameIndex;
    quint16_le length;
    quint16_le nFormals;
    quint32_le formalsOffset; // Can't turn this into a calculated offset because of the mutation in CompilationUnit::createUnitData.
    ParameterType returnType;
    quint32_le localsOffset;
    quint16_le nLocals;
    quint16_le nLineNumbers;
    size_t lineNumberOffset() const { return localsOffset + nLocals * sizeof(quint32); }
    quint32_le nestedFunctionIndex; // for functions that only return a single closure, used in signal handlers

    quint32_le nRegisters;
    Location location;
    quint32_le nLabelInfos;

    quint16_le sizeOfLocalTemporalDeadZone;
    quint16_le firstTemporalDeadZoneRegister;
    quint16_le sizeOfRegisterTemporalDeadZone;

    size_t labelInfosOffset() const { return lineNumberOffset() + nLineNumbers * sizeof(CodeOffsetToLine); }

    // Keep all unaligned data at the end
    quint8 flags;
    quint8 padding1;

    //    quint32 formalsIndex[nFormals]
    //    quint32 localsIndex[nLocals]

    const Parameter *formalsTable() const { return reinterpret_cast<const Parameter *>(reinterpret_cast<const char *>(this) + formalsOffset); }
    const quint32_le *localsTable() const { return reinterpret_cast<const quint32_le *>(reinterpret_cast<const char *>(this) + localsOffset); }
    const CodeOffsetToLine *lineNumberTable() const { return reinterpret_cast<const CodeOffsetToLine *>(reinterpret_cast<const char *>(this) + lineNumberOffset()); }

    // --- QQmlPropertyCacheCreator interface
    const Parameter *formalsBegin() const { return formalsTable(); }
    const Parameter *formalsEnd() const { return formalsTable() + nFormals; }
    // ---

    const quint32_le *labelInfoTable() const { return reinterpret_cast<const quint32_le *>(reinterpret_cast<const char *>(this) + labelInfosOffset()); }

    const char *code() const { return reinterpret_cast<const char *>(this) + codeOffset; }

    static int calculateSize(int nFormals, int nLocals, int nLines, int nInnerfunctions, int labelInfoSize, int codeSize) {
        int trailingData = nFormals * sizeof(Parameter) + (nLocals + nInnerfunctions + labelInfoSize)*sizeof (quint32)
                + nLines*sizeof(CodeOffsetToLine);
        size_t size = align(align(sizeof(Function)) + size_t(trailingData)) + align(codeSize);
        Q_ASSERT(size < INT_MAX);
        return int(size);
    }

    static size_t align(size_t a) {
        return (a + 7) & ~size_t(7);
    }
};
static_assert(sizeof(Function) == 56, "Function structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct Method {
    enum Type {
        Regular,
        Getter,
        Setter
    };

    quint32_le name;
    quint32_le type;
    quint32_le function;
};
static_assert(sizeof(Method) == 12, "Method structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct Class
{
    quint32_le nameIndex;
    quint32_le scopeIndex;
    quint32_le constructorFunction;
    quint32_le nStaticMethods;
    quint32_le nMethods;
    quint32_le methodTableOffset;

    const Method *methodTable() const { return reinterpret_cast<const Method *>(reinterpret_cast<const char *>(this) + methodTableOffset); }

    static int calculateSize(int nStaticMethods, int nMethods) {
        int trailingData = (nStaticMethods + nMethods) * sizeof(Method);
        size_t size = align(sizeof(Class) + trailingData);
        Q_ASSERT(size < INT_MAX);
        return int(size);
    }

    static size_t align(size_t a) {
        return (a + 7) & ~size_t(7);
    }
};
static_assert(sizeof(Class) == 24, "Class structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct TemplateObject
{
    quint32_le size;

    static int calculateSize(int size) {
        int trailingData = 2 * size * sizeof(quint32_le);
        size_t s = align(sizeof(TemplateObject) + trailingData);
        Q_ASSERT(s < INT_MAX);
        return int(s);
    }

    static size_t align(size_t a) {
        return (a + 7) & ~size_t(7);
    }

    const quint32_le *stringTable() const {
        return reinterpret_cast<const quint32_le *>(reinterpret_cast<const char *>(this + 1));
    }

    uint stringIndexAt(uint i) const {
        return stringTable()[i];
    }
    uint rawStringIndexAt(uint i) const {
        return stringTable()[size + i];
    }
};
static_assert(sizeof(TemplateObject) == 4, "Template object structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct ExportEntry
{
    quint32_le exportName;
    quint32_le moduleRequest;
    quint32_le importName;
    quint32_le localName;
    Location location;
};
static_assert(sizeof(ExportEntry) == 20, "ExportEntry structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct ImportEntry
{
    quint32_le moduleRequest;
    quint32_le importName;
    quint32_le localName;
    Location location;
};
static_assert(sizeof(ImportEntry) == 16, "ImportEntry structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

// Qml data structures

struct TranslationData
{
    quint32_le stringIndex;
    quint32_le commentIndex;
    qint32_le number;
    quint32_le padding;
};
static_assert(sizeof(TranslationData) == 16, "TranslationData structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct Binding
{
    quint32_le propertyNameIndex;

    enum Type : unsigned int {
        Type_Invalid,
        Type_Boolean,
        Type_Number,
        Type_String,
        Type_Null,
        Type_Translation,
        Type_TranslationById,
        Type_Script,
        Type_Object,
        Type_AttachedProperty,
        Type_GroupProperty
    };

    enum Flag : unsigned int {
        IsSignalHandlerExpression = 0x1,
        IsSignalHandlerObject = 0x2,
        IsOnAssignment = 0x4,
        InitializerForReadOnlyDeclaration = 0x8,
        IsResolvedEnum = 0x10,
        IsListItem = 0x20,
        IsBindingToAlias = 0x40,
        IsDeferredBinding = 0x80,
        IsCustomParserBinding = 0x100,
        IsFunctionExpression = 0x200,
        IsPropertyObserver = 0x400
    };
    Q_DECLARE_FLAGS(Flags, Flag);

    using FlagsField = quint32_le_bitfield_member<0, 16>;
    using TypeField = quint32_le_bitfield_member<16, 16>;
    quint32_le_bitfield_union<FlagsField, TypeField> flagsAndType;

    void clearFlags() { flagsAndType.set<FlagsField>(0); }
    void setFlag(Flag flag) { flagsAndType.set<FlagsField>(flagsAndType.get<FlagsField>() | flag); }
    bool hasFlag(Flag flag) const { return Flags(flagsAndType.get<FlagsField>()) & flag; }
    Flags flags() const { return Flags(flagsAndType.get<FlagsField>()); }

    void setType(Type type) { flagsAndType.set<TypeField>(type); }
    Type type() const { return Type(flagsAndType.get<TypeField>()); }

    union {
        bool b;
        quint32_le constantValueIndex;
        quint32_le compiledScriptIndex; // used when Type_Script
        quint32_le objectIndex;
        quint32_le translationDataIndex; // used when Type_Translation
        quint32 nullMarker;
    } value;
    quint32_le stringIndex; // Set for Type_String and Type_Script (the latter because of script strings)

    Location location;
    Location valueLocation;

    bool hasSignalHandlerBindingFlag() const
    {
        const Flags bindingFlags = flags();
        return bindingFlags & IsSignalHandlerExpression
                || bindingFlags & IsSignalHandlerObject
                || bindingFlags & IsPropertyObserver;
    }

    bool isValueBinding() const
    {
        switch (type()) {
        case Type_AttachedProperty:
        case Type_GroupProperty:
            return false;
        default:
            return !hasSignalHandlerBindingFlag();
        }
    }

    bool isValueBindingNoAlias() const { return isValueBinding() && !hasFlag(IsBindingToAlias); }
    bool isValueBindingToAlias() const { return isValueBinding() && hasFlag(IsBindingToAlias); }

    bool isSignalHandler() const
    {
        if (hasSignalHandlerBindingFlag()) {
            Q_ASSERT(!isValueBinding());
            Q_ASSERT(!isAttachedProperty());
            Q_ASSERT(!isGroupProperty());
            return true;
        }
        return false;
    }

    bool isAttachedProperty() const
    {
        if (type() == Type_AttachedProperty) {
            Q_ASSERT(!isValueBinding());
            Q_ASSERT(!isSignalHandler());
            Q_ASSERT(!isGroupProperty());
            return true;
        }
        return false;
    }

    bool isGroupProperty() const
    {
        if (type() == Type_GroupProperty) {
            Q_ASSERT(!isValueBinding());
            Q_ASSERT(!isSignalHandler());
            Q_ASSERT(!isAttachedProperty());
            return true;
        }
        return false;
    }

    bool isFunctionExpression() const { return hasFlag(IsFunctionExpression); }

    //reverse of Lexer::singleEscape()
    static QString escapedString(const QString &string)
    {
        QString tmp = QLatin1String("\"");
        for (int i = 0; i < string.length(); ++i) {
            const QChar &c = string.at(i);
            switch (c.unicode()) {
            case 0x08:
                tmp += QLatin1String("\\b");
                break;
            case 0x09:
                tmp += QLatin1String("\\t");
                break;
            case 0x0A:
                tmp += QLatin1String("\\n");
                break;
            case 0x0B:
                tmp += QLatin1String("\\v");
                break;
            case 0x0C:
                tmp += QLatin1String("\\f");
                break;
            case 0x0D:
                tmp += QLatin1String("\\r");
                break;
            case 0x22:
                tmp += QLatin1String("\\\"");
                break;
            case 0x27:
                tmp += QLatin1String("\\\'");
                break;
            case 0x5C:
                tmp += QLatin1String("\\\\");
                break;
            default:
                tmp += c;
                break;
            }
        }
        tmp += QLatin1Char('\"');
        return tmp;
    }

    bool isTranslationBinding() const
    {
        const Binding::Type bindingType = type();
        return bindingType == Type_Translation || bindingType == Type_TranslationById;
    }
    bool evaluatesToString() const { return type() == Type_String || isTranslationBinding(); }

    bool isNumberBinding() const { return type() == Type_Number; }

    bool valueAsBoolean() const
    {
        if (type() == Type_Boolean)
            return value.b;
        return false;
    }
};

static_assert(sizeof(Binding) == 24, "Binding structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct InlineComponent
{
    quint32_le objectIndex;
    quint32_le nameIndex;
    Location location;
};

static_assert(sizeof(InlineComponent) == 12, "InlineComponent structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct EnumValue
{
    quint32_le nameIndex;
    qint32_le value;
    Location location;
};
static_assert(sizeof(EnumValue) == 12, "EnumValue structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct Enum
{
    quint32_le nameIndex;
    quint32_le nEnumValues;
    Location location;

    const EnumValue *enumValueAt(int idx) const {
        return reinterpret_cast<const EnumValue*>(this + 1) + idx;
    }

    static int calculateSize(int nEnumValues) {
        return (sizeof(Enum)
                + nEnumValues * sizeof(EnumValue)
                + 7) & ~0x7;
    }

    // --- QQmlPropertyCacheCreatorInterface
    const EnumValue *enumValuesBegin() const { return enumValueAt(0); }
    const EnumValue *enumValuesEnd() const { return enumValueAt(nEnumValues); }
    int enumValueCount() const { return nEnumValues; }
    // ---
};
static_assert(sizeof(Enum) == 12, "Enum structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct Signal
{
    quint32_le nameIndex;
    quint32_le nParameters;
    Location location;
    // Parameter parameters[1];

    const Parameter *parameterAt(int idx) const {
        return reinterpret_cast<const Parameter*>(this + 1) + idx;
    }

    static int calculateSize(int nParameters) {
        return (sizeof(Signal)
                + nParameters * sizeof(Parameter)
                + 7) & ~0x7;
    }

    // --- QQmlPropertyCacheCceatorInterface
    const Parameter *parametersBegin() const { return parameterAt(0); }
    const Parameter *parametersEnd() const { return parameterAt(nParameters); }
    int parameterCount() const { return nParameters; }
    // ---
};
static_assert(sizeof(Signal) == 12, "Signal structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct Property
{
private:
    using BuiltinTypeOrTypeNameIndexField = quint32_le_bitfield_member<0, 28>;
    using IsRequiredField = quint32_le_bitfield_member<28, 1>;
    using IsBuiltinTypeField = quint32_le_bitfield_member<29, 1>;
    using IsListField = quint32_le_bitfield_member<30, 1>;
    using IsReadOnlyField = quint32_le_bitfield_member<31, 1>;

public:
    quint32_le nameIndex;
    quint32_le_bitfield_union<
            BuiltinTypeOrTypeNameIndexField,
            IsRequiredField,
            IsBuiltinTypeField,
            IsListField,
            IsReadOnlyField> data;
    Location location;

    void setBuiltinType(BuiltinType t)
    {
        data.set<BuiltinTypeOrTypeNameIndexField>(static_cast<quint32>(t));
        data.set<IsBuiltinTypeField>(true);
    }

    BuiltinType builtinType() const {
        if (data.get<IsBuiltinTypeField>() != 0)
            return BuiltinType(data.get<BuiltinTypeOrTypeNameIndexField>());
        return BuiltinType::InvalidBuiltin;
    }

    void setCustomType(int nameIndex)
    {
        data.set<BuiltinTypeOrTypeNameIndexField>(nameIndex);
        data.set<IsBuiltinTypeField>(false);
    }

    int customType() const
    {
        return data.get<IsBuiltinTypeField>() ? -1 : data.get<BuiltinTypeOrTypeNameIndexField>();
    }

    bool isBuiltinType() const { return data.get<IsBuiltinTypeField>(); }
    uint builtinTypeOrTypeNameIndex() const { return data.get<BuiltinTypeOrTypeNameIndexField>(); }

    bool isList() const { return data.get<IsListField>(); }
    void setIsList(bool isList) { data.set<IsListField>(isList); }

    bool isRequired() const { return data.get<IsRequiredField>(); }
    void setIsRequired(bool isRequired) { data.set<IsRequiredField>(isRequired); }

    bool isReadOnly() const { return data.get<IsReadOnlyField>(); }
    void setIsReadOnly(bool isReadOnly) { data.set<IsReadOnlyField>(isReadOnly); }
};
static_assert(sizeof(Property) == 12, "Property structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct RequiredPropertyExtraData {
    quint32_le nameIndex;
};

static_assert (sizeof(RequiredPropertyExtraData) == 4, "RequiredPropertyExtraData structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct Alias {
private:
    using NameIndexField = quint32_le_bitfield_member<0, 29>;
    using FlagsField = quint32_le_bitfield_member<29, 3>;

    // object id index (in QQmlContextData::idValues)
    using TargetObjectIdField = quint32_le_bitfield_member<0, 31>;
    using AliasToLocalAliasField = quint32_le_bitfield_member<31, 1>;

public:

    enum Flag : unsigned int {
        IsReadOnly = 0x1,
        Resolved = 0x2,
        AliasPointsToPointerObject = 0x4
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    quint32_le_bitfield_union<NameIndexField, FlagsField> nameIndexAndFlags;

    union {
        quint32_le idIndex; // string index
        quint32_le_bitfield_union<TargetObjectIdField, AliasToLocalAliasField>
                targetObjectIdAndAliasToLocalAlias;
    };

    union {
        quint32_le propertyNameIndex; // string index
        qint32_le encodedMetaPropertyIndex;
        quint32_le localAliasIndex; // index in list of aliases local to the object (if targetObjectId == objectId)
    };
    Location location;
    Location referenceLocation;

    bool hasFlag(Flag flag) const
    {
        return nameIndexAndFlags.get<FlagsField>() & flag;
    }

    void setFlag(Flag flag)
    {
        nameIndexAndFlags.set<FlagsField>(nameIndexAndFlags.get<FlagsField>() | flag);
    }

    void clearFlags()
    {
        nameIndexAndFlags.set<FlagsField>(0);
    }

    quint32 nameIndex() const
    {
        return nameIndexAndFlags.get<NameIndexField>();
    }

    void setNameIndex(quint32 nameIndex)
    {
        nameIndexAndFlags.set<NameIndexField>(nameIndex);
    }

    bool isObjectAlias() const
    {
        Q_ASSERT(hasFlag(Resolved));
        return encodedMetaPropertyIndex == -1;
    }

    bool isAliasToLocalAlias() const
    {
        return targetObjectIdAndAliasToLocalAlias.get<AliasToLocalAliasField>();
    }

    void setIsAliasToLocalAlias(bool isAliasToLocalAlias)
    {
        targetObjectIdAndAliasToLocalAlias.set<AliasToLocalAliasField>(isAliasToLocalAlias);
    }

    quint32 targetObjectId() const
    {
        return targetObjectIdAndAliasToLocalAlias.get<TargetObjectIdField>();
    }

    void setTargetObjectId(quint32 targetObjectId)
    {
        targetObjectIdAndAliasToLocalAlias.set<TargetObjectIdField>(targetObjectId);
    }
};
static_assert(sizeof(Alias) == 20, "Alias structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct Object
{
private:
    using FlagsField = quint32_le_bitfield_member<0, 15>;
    using DefaultPropertyIsAliasField = quint32_le_bitfield_member<15, 1>;
    using IdField = quint32_le_bitfield_member<16, 16, qint32>;
public:
    enum Flag : unsigned int {
        NoFlag = 0x0,
        IsComponent = 0x1, // object was identified to be an explicit or implicit component boundary
        HasDeferredBindings = 0x2, // any of the bindings are deferred
        HasCustomParserBindings = 0x4,
        IsInlineComponentRoot = 0x8,
        InPartOfInlineComponent = 0x10
    };
    Q_DECLARE_FLAGS(Flags, Flag);

    // Depending on the use, this may be the type name to instantiate before instantiating this
    // object. For grouped properties the type name will be empty and for attached properties
    // it will be the name of the attached type.
    quint32_le inheritedTypeNameIndex;
    quint32_le idNameIndex;
    quint32_le_bitfield_union<FlagsField, DefaultPropertyIsAliasField, IdField>
            flagsAndDefaultPropertyIsAliasAndId;
    qint32_le indexOfDefaultPropertyOrAlias; // -1 means no default property declared in this object
    quint16_le nFunctions;
    quint16_le nProperties;
    quint32_le offsetToFunctions;
    quint32_le offsetToProperties;
    quint32_le offsetToAliases;
    quint16_le nAliases;
    quint16_le nEnums;
    quint32_le offsetToEnums; // which in turn will be a table with offsets to variable-sized Enum objects
    quint32_le offsetToSignals; // which in turn will be a table with offsets to variable-sized Signal objects
    quint16_le nSignals;
    quint16_le nBindings;
    quint32_le offsetToBindings;
    quint32_le nNamedObjectsInComponent;
    quint32_le offsetToNamedObjectsInComponent;
    Location location;
    Location locationOfIdProperty;
    quint32_le offsetToInlineComponents;
    quint16_le nInlineComponents;
    quint32_le offsetToRequiredPropertyExtraData;
    quint16_le nRequiredPropertyExtraData;
//    Function[]
//    Property[]
//    Signal[]
//    Binding[]
//    InlineComponent[]
//    RequiredPropertyExtraData[]

    Flags flags() const
    {
        return Flags(flagsAndDefaultPropertyIsAliasAndId.get<FlagsField>());
    }

    bool hasFlag(Flag flag) const
    {
        return flagsAndDefaultPropertyIsAliasAndId.get<FlagsField>() & flag;
    }

    void setFlag(Flag flag)
    {
        flagsAndDefaultPropertyIsAliasAndId.set<FlagsField>(
                flagsAndDefaultPropertyIsAliasAndId.get<FlagsField>() | flag);
    }

    void setFlags(Flags flags)
    {
        flagsAndDefaultPropertyIsAliasAndId.set<FlagsField>(flags);
    }

    bool hasAliasAsDefaultProperty() const
    {
        return flagsAndDefaultPropertyIsAliasAndId.get<DefaultPropertyIsAliasField>();
    }

    void setHasAliasAsDefaultProperty(bool defaultAlias)
    {
        flagsAndDefaultPropertyIsAliasAndId.set<DefaultPropertyIsAliasField>(defaultAlias);
    }

    qint32 objectId() const
    {
        return flagsAndDefaultPropertyIsAliasAndId.get<IdField>();
    }

    void setObjectId(qint32 id)
    {
        flagsAndDefaultPropertyIsAliasAndId.set<IdField>(id);
    }


    static int calculateSizeExcludingSignalsAndEnums(int nFunctions, int nProperties, int nAliases, int nEnums, int nSignals, int nBindings, int nNamedObjectsInComponent, int nInlineComponents, int nRequiredPropertyExtraData)
    {
        return ( sizeof(Object)
                 + nFunctions * sizeof(quint32)
                 + nProperties * sizeof(Property)
                 + nAliases * sizeof(Alias)
                 + nEnums * sizeof(quint32)
                 + nSignals * sizeof(quint32)
                 + nBindings * sizeof(Binding)
                 + nNamedObjectsInComponent * sizeof(int)
                 + nInlineComponents * sizeof(InlineComponent)
                 + nRequiredPropertyExtraData * sizeof(RequiredPropertyExtraData)
                 + 0x7
               ) & ~0x7;
    }

    const quint32_le *functionOffsetTable() const
    {
        return reinterpret_cast<const quint32_le*>(reinterpret_cast<const char *>(this) + offsetToFunctions);
    }

    const Property *propertyTable() const
    {
        return reinterpret_cast<const Property*>(reinterpret_cast<const char *>(this) + offsetToProperties);
    }

    const Alias *aliasTable() const
    {
        return reinterpret_cast<const Alias*>(reinterpret_cast<const char *>(this) + offsetToAliases);
    }

    const Binding *bindingTable() const
    {
        return reinterpret_cast<const Binding*>(reinterpret_cast<const char *>(this) + offsetToBindings);
    }

    const Enum *enumAt(int idx) const
    {
        const quint32_le *offsetTable = reinterpret_cast<const quint32_le*>((reinterpret_cast<const char *>(this)) + offsetToEnums);
        const quint32_le offset = offsetTable[idx];
        return reinterpret_cast<const Enum*>(reinterpret_cast<const char*>(this) + offset);
    }

    const Signal *signalAt(int idx) const
    {
        const quint32_le *offsetTable = reinterpret_cast<const quint32_le*>((reinterpret_cast<const char *>(this)) + offsetToSignals);
        const quint32_le offset = offsetTable[idx];
        return reinterpret_cast<const Signal*>(reinterpret_cast<const char*>(this) + offset);
    }

    const InlineComponent *inlineComponentAt(int idx) const
    {
        return inlineComponentTable() + idx;
    }

    const quint32_le *namedObjectsInComponentTable() const
    {
        return reinterpret_cast<const quint32_le*>(reinterpret_cast<const char *>(this) + offsetToNamedObjectsInComponent);
    }

    const InlineComponent *inlineComponentTable() const
    {
        return reinterpret_cast<const InlineComponent*>(reinterpret_cast<const char *>(this) + offsetToInlineComponents);
    }

    const RequiredPropertyExtraData *requiredPropertyExtraDataAt(int idx) const
    {
        return requiredPropertyExtraDataTable() + idx;
    }

    const RequiredPropertyExtraData *requiredPropertyExtraDataTable() const
    {
        return reinterpret_cast<const RequiredPropertyExtraData*>(reinterpret_cast<const char *>(this) + offsetToRequiredPropertyExtraData);
    }

    // --- QQmlPropertyCacheCreator interface
    int propertyCount() const { return nProperties; }
    int aliasCount() const { return nAliases; }
    int enumCount() const { return nEnums; }
    int signalCount() const { return nSignals; }
    int functionCount() const { return nFunctions; }

    const Binding *bindingsBegin() const { return bindingTable(); }
    const Binding *bindingsEnd() const { return bindingTable() + nBindings; }

    const Property *propertiesBegin() const { return propertyTable(); }
    const Property *propertiesEnd() const { return propertyTable() + nProperties; }

    const Alias *aliasesBegin() const { return aliasTable(); }
    const Alias *aliasesEnd() const { return aliasTable() + nAliases; }

    typedef TableIterator<Enum, Object, &Object::enumAt> EnumIterator;
    EnumIterator enumsBegin() const { return EnumIterator(this, 0); }
    EnumIterator enumsEnd() const { return EnumIterator(this, nEnums); }

    typedef TableIterator<Signal, Object, &Object::signalAt> SignalIterator;
    SignalIterator signalsBegin() const { return SignalIterator(this, 0); }
    SignalIterator signalsEnd() const { return SignalIterator(this, nSignals); }

    typedef TableIterator<InlineComponent, Object, &Object::inlineComponentAt> InlineComponentIterator;
    InlineComponentIterator inlineComponentsBegin() const {return InlineComponentIterator(this, 0);}
    InlineComponentIterator inlineComponentsEnd() const {return InlineComponentIterator(this, nInlineComponents);}

    typedef TableIterator<RequiredPropertyExtraData, Object, &Object::requiredPropertyExtraDataAt> RequiredPropertyExtraDataIterator;
    RequiredPropertyExtraDataIterator requiredPropertyExtraDataBegin() const {return RequiredPropertyExtraDataIterator(this, 0); };
    RequiredPropertyExtraDataIterator requiredPropertyExtraDataEnd() const {return RequiredPropertyExtraDataIterator(this, nRequiredPropertyExtraData); };

    int namedObjectsInComponentCount() const { return nNamedObjectsInComponent; }
    // ---
};
static_assert(sizeof(Object) == 84, "Object structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct Import
{
    enum ImportType : unsigned int {
        ImportLibrary = 0x1,
        ImportFile = 0x2,
        ImportScript = 0x3,
        ImportInlineComponent = 0x4
    };
    quint32_le type;

    quint32_le uriIndex;
    quint32_le qualifierIndex;

    Location location;
    QTypeRevision version;
    quint16_le reserved;

    Import()
    {
        type = 0; uriIndex = 0; qualifierIndex = 0; version = QTypeRevision::zero(); reserved = 0;
    }
};
static_assert(sizeof(Import) == 20, "Import structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct QmlUnit
{
    quint32_le nImports;
    quint32_le offsetToImports;
    quint32_le nObjects;
    quint32_le offsetToObjects;

    const Import *importAt(int idx) const {
        return reinterpret_cast<const Import*>((reinterpret_cast<const char *>(this)) + offsetToImports + idx * sizeof(Import));
    }

    const Object *objectAt(int idx) const {
        const quint32_le *offsetTable = reinterpret_cast<const quint32_le*>((reinterpret_cast<const char *>(this)) + offsetToObjects);
        const quint32_le offset = offsetTable[idx];
        return reinterpret_cast<const Object*>(reinterpret_cast<const char*>(this) + offset);
    }
};
static_assert(sizeof(QmlUnit) == 16, "QmlUnit structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

enum { QmlCompileHashSpace = 48 };
static const char magic_str[] = "qv4cdata";

struct Unit
{
    // DO NOT CHANGE THESE FIELDS EVER
    char magic[8];
    quint32_le version;
    quint32_le qtVersion;
    qint64_le sourceTimeStamp;
    quint32_le unitSize; // Size of the Unit and any depending data.
    // END DO NOT CHANGE THESE FIELDS EVER

    char libraryVersionHash[QmlCompileHashSpace];

    char md5Checksum[16]; // checksum of all bytes following this field.
    char dependencyMD5Checksum[16];

    enum : unsigned int {
        IsJavascript = 0x1,
        StaticData = 0x2, // Unit data persistent in memory?
        IsSingleton = 0x4,
        IsSharedLibrary = 0x8, // .pragma shared?
        IsESModule = 0x10,
        PendingTypeCompilation = 0x20, // the QML data structures present are incomplete and require type compilation
        IsStrict = 0x40
    };
    quint32_le flags;
    quint32_le stringTableSize;
    quint32_le offsetToStringTable;
    quint32_le functionTableSize;
    quint32_le offsetToFunctionTable;
    quint32_le classTableSize;
    quint32_le offsetToClassTable;
    quint32_le templateObjectTableSize;
    quint32_le offsetToTemplateObjectTable;
    quint32_le blockTableSize;
    quint32_le offsetToBlockTable;
    quint32_le lookupTableSize;
    quint32_le offsetToLookupTable;
    quint32_le regexpTableSize;
    quint32_le offsetToRegexpTable;
    quint32_le constantTableSize;
    quint32_le offsetToConstantTable;
    quint32_le jsClassTableSize;
    quint32_le offsetToJSClassTable;
    quint32_le translationTableSize;
    quint32_le offsetToTranslationTable;
    quint32_le localExportEntryTableSize;
    quint32_le offsetToLocalExportEntryTable;
    quint32_le indirectExportEntryTableSize;
    quint32_le offsetToIndirectExportEntryTable;
    quint32_le starExportEntryTableSize;
    quint32_le offsetToStarExportEntryTable;
    quint32_le importEntryTableSize;
    quint32_le offsetToImportEntryTable;
    quint32_le moduleRequestTableSize;
    quint32_le offsetToModuleRequestTable;
    qint32_le indexOfRootFunction;
    quint32_le sourceFileIndex;
    quint32_le finalUrlIndex;

    quint32_le offsetToQmlUnit;

    /* QML specific fields */

    const QmlUnit *qmlUnit() const {
        return reinterpret_cast<const QmlUnit *>(reinterpret_cast<const char *>(this) + offsetToQmlUnit);
    }

    QmlUnit *qmlUnit() {
        return reinterpret_cast<QmlUnit *>(reinterpret_cast<char *>(this) + offsetToQmlUnit);
    }

    bool isSingleton() const {
        return flags & Unit::IsSingleton;
    }
    /* end QML specific fields*/

    QString stringAtInternal(int idx) const {
        Q_ASSERT(idx < int(stringTableSize));
        const quint32_le *offsetTable = reinterpret_cast<const quint32_le*>((reinterpret_cast<const char *>(this)) + offsetToStringTable);
        const quint32_le offset = offsetTable[idx];
        const String *str = reinterpret_cast<const String*>(reinterpret_cast<const char *>(this) + offset);
        Q_ASSERT(str->size >= 0);
        if (str->size == 0)
            return QString();
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
        const QChar *characters = reinterpret_cast<const QChar *>(str + 1);
        if (flags & StaticData)
            return QString::fromRawData(characters, str->size);
        return QString(characters, str->size);
#else
        const quint16_le *characters = reinterpret_cast<const quint16_le *>(str + 1);
        QString qstr(str->size, Qt::Uninitialized);
        QChar *ch = qstr.data();
        for (int i = 0; i < str->size; ++i)
             ch[i] = QChar(characters[i]);
         return qstr;
#endif
    }

    const quint32_le *functionOffsetTable() const { return reinterpret_cast<const quint32_le*>((reinterpret_cast<const char *>(this)) + offsetToFunctionTable); }
    const quint32_le *classOffsetTable() const { return reinterpret_cast<const quint32_le*>((reinterpret_cast<const char *>(this)) + offsetToClassTable); }
    const quint32_le *templateObjectOffsetTable() const { return reinterpret_cast<const quint32_le*>((reinterpret_cast<const char *>(this)) + offsetToTemplateObjectTable); }
    const quint32_le *blockOffsetTable() const { return reinterpret_cast<const quint32_le*>((reinterpret_cast<const char *>(this)) + offsetToBlockTable); }

    const Function *functionAt(int idx) const {
        const quint32_le *offsetTable = functionOffsetTable();
        const quint32_le offset = offsetTable[idx];
        return reinterpret_cast<const Function*>(reinterpret_cast<const char *>(this) + offset);
    }

    const Class *classAt(int idx) const {
        const quint32_le *offsetTable = classOffsetTable();
        const quint32_le offset = offsetTable[idx];
        return reinterpret_cast<const Class *>(reinterpret_cast<const char *>(this) + offset);
    }

    const TemplateObject *templateObjectAt(int idx) const {
        const quint32_le *offsetTable = templateObjectOffsetTable();
        const quint32_le offset = offsetTable[idx];
        return reinterpret_cast<const TemplateObject *>(reinterpret_cast<const char *>(this) + offset);
    }

    const Block *blockAt(int idx) const {
        const quint32_le *offsetTable = blockOffsetTable();
        const quint32_le offset = offsetTable[idx];
        return reinterpret_cast<const Block *>(reinterpret_cast<const char *>(this) + offset);
    }

    const Lookup *lookupTable() const { return reinterpret_cast<const Lookup*>(reinterpret_cast<const char *>(this) + offsetToLookupTable); }
    const RegExp *regexpAt(int index) const {
        return reinterpret_cast<const RegExp*>(reinterpret_cast<const char *>(this) + offsetToRegexpTable + index * sizeof(RegExp));
    }
    const quint64_le *constants() const {
        return reinterpret_cast<const quint64_le*>(reinterpret_cast<const char *>(this) + offsetToConstantTable);
    }

    const JSClassMember *jsClassAt(int idx, int *nMembers) const {
        const quint32_le *offsetTable = reinterpret_cast<const quint32_le *>(reinterpret_cast<const char *>(this) + offsetToJSClassTable);
        const quint32_le offset = offsetTable[idx];
        const char *ptr = reinterpret_cast<const char *>(this) + offset;
        const JSClass *klass = reinterpret_cast<const JSClass *>(ptr);
        *nMembers = klass->nMembers;
        return reinterpret_cast<const JSClassMember*>(ptr + sizeof(JSClass));
    }

    const TranslationData *translations() const {
        return reinterpret_cast<const TranslationData *>(reinterpret_cast<const char *>(this) + offsetToTranslationTable);
    }

    const ImportEntry *importEntryTable() const { return reinterpret_cast<const ImportEntry *>(reinterpret_cast<const char *>(this) + offsetToImportEntryTable); }
    const ExportEntry *localExportEntryTable() const { return reinterpret_cast<const ExportEntry *>(reinterpret_cast<const char *>(this) + offsetToLocalExportEntryTable); }
    const ExportEntry *indirectExportEntryTable() const { return reinterpret_cast<const ExportEntry *>(reinterpret_cast<const char *>(this) + offsetToIndirectExportEntryTable); }
    const ExportEntry *starExportEntryTable() const { return reinterpret_cast<const ExportEntry *>(reinterpret_cast<const char *>(this) + offsetToStarExportEntryTable); }

    const quint32_le *moduleRequestTable() const { return reinterpret_cast<const quint32_le*>((reinterpret_cast<const char *>(this)) + offsetToModuleRequestTable); }
};

static_assert(sizeof(Unit) == 248, "Unit structure needs to have the expected size to be binary compatible on disk when generated by host compiler and loaded by target");

struct TypeReference
{
    TypeReference(const Location &loc)
        : location(loc)
        , needsCreation(false)
        , errorWhenNotFound(false)
    {}
    Location location; // first use
    bool needsCreation : 1; // whether the type needs to be creatable or not
    bool errorWhenNotFound: 1;
};

// Map from name index to location of first use.
struct TypeReferenceMap : QHash<int, TypeReference>
{
    TypeReference &add(int nameIndex, const Location &loc) {
        Iterator it = find(nameIndex);
        if (it != end())
            return *it;
        return *insert(nameIndex, loc);
    }

    template <typename Iterator>
    void collectFromFunctions(Iterator it, Iterator end)
    {
        for (; it != end; ++it) {
            auto formal = it->formalsBegin();
            auto formalEnd = it->formalsEnd();
            for ( ; formal != formalEnd; ++formal) {
                if (!formal->type.indexIsBuiltinType()) {
                    TypeReference &r
                            = this->add(formal->type.typeNameIndexOrBuiltinType(), it->location);
                    r.errorWhenNotFound = true;
                }
            }

            if (!it->returnType.indexIsBuiltinType()) {
                TypeReference &r
                    = this->add(it->returnType.typeNameIndexOrBuiltinType(), it->location);
                r.errorWhenNotFound = true;
            }
        }
    }

    template <typename CompiledObject>
    void collectFromObject(const CompiledObject *obj)
    {
        if (obj->inheritedTypeNameIndex != 0) {
            TypeReference &r = this->add(obj->inheritedTypeNameIndex, obj->location);
            r.needsCreation = true;
            r.errorWhenNotFound = true;
        }

        auto prop = obj->propertiesBegin();
        auto const propEnd = obj->propertiesEnd();
        for ( ; prop != propEnd; ++prop) {
            if (!prop->isBuiltinType()) {
                TypeReference &r = this->add(prop->builtinTypeOrTypeNameIndex(), prop->location);
                r.errorWhenNotFound = true;
            }
        }

        auto binding = obj->bindingsBegin();
        auto const bindingEnd = obj->bindingsEnd();
        for ( ; binding != bindingEnd; ++binding) {
            if (binding->type() == QV4::CompiledData::Binding::Type_AttachedProperty)
                this->add(binding->propertyNameIndex, binding->location);
        }

        auto ic = obj->inlineComponentsBegin();
        auto const icEnd = obj->inlineComponentsEnd();
        for (; ic != icEnd; ++ic) {
            this->add(ic->nameIndex, ic->location);
        }
    }
};

using DependentTypesHasher = std::function<QByteArray()>;

// This is how this hooks into the existing structures:

struct CompilationUnitBase
{
    Q_DISABLE_COPY(CompilationUnitBase)

    CompilationUnitBase() = default;
    ~CompilationUnitBase() = default;

    CompilationUnitBase(CompilationUnitBase &&other) noexcept { *this = std::move(other); }

    CompilationUnitBase &operator=(CompilationUnitBase &&other) noexcept
    {
        if (this != &other) {
            runtimeStrings = other.runtimeStrings;
            other.runtimeStrings = nullptr;
            constants = other.constants;
            other.constants = nullptr;
            runtimeRegularExpressions = other.runtimeRegularExpressions;
            other.runtimeRegularExpressions = nullptr;
            runtimeClasses = other.runtimeClasses;
            other.runtimeClasses = nullptr;
            imports = other.imports;
            other.imports = nullptr;
        }
        return *this;
    }

    // pointers either to data->constants() or little-endian memory copy.
    Heap::String **runtimeStrings = nullptr; // Array
    const StaticValue* constants = nullptr;
    QV4::StaticValue *runtimeRegularExpressions = nullptr;
    Heap::InternalClass **runtimeClasses = nullptr;
    const StaticValue** imports = nullptr;
};

Q_STATIC_ASSERT(std::is_standard_layout<CompilationUnitBase>::value);
Q_STATIC_ASSERT(offsetof(CompilationUnitBase, runtimeStrings) == 0);
Q_STATIC_ASSERT(offsetof(CompilationUnitBase, constants) == sizeof(QV4::Heap::String **));
Q_STATIC_ASSERT(offsetof(CompilationUnitBase, runtimeRegularExpressions) == offsetof(CompilationUnitBase, constants) + sizeof(const StaticValue *));
Q_STATIC_ASSERT(offsetof(CompilationUnitBase, runtimeClasses) == offsetof(CompilationUnitBase, runtimeRegularExpressions) + sizeof(const StaticValue *));
Q_STATIC_ASSERT(offsetof(CompilationUnitBase, imports) == offsetof(CompilationUnitBase, runtimeClasses) + sizeof(const StaticValue *));

struct CompilationUnit : public CompilationUnitBase
{
    Q_DISABLE_COPY(CompilationUnit)

    const Unit *data = nullptr;
    const QmlUnit *qmlData = nullptr;
    QStringList dynamicStrings;
    const QQmlPrivate::AOTCompiledFunction *aotCompiledFunctions = nullptr;
public:
    using CompiledObject = CompiledData::Object;

    CompilationUnit(const Unit *unitData = nullptr, const QString &fileName = QString(),
                    const QString &finalUrlString = QString())
    {
        setUnitData(unitData, nullptr, fileName, finalUrlString);
    }

    explicit CompilationUnit(const Unit *unitData, const QQmlPrivate::AOTCompiledFunction *aotCompiledFunctions,
                             const QString &fileName = QString(), const QString &finalUrlString = QString())
        : CompilationUnit(unitData, fileName, finalUrlString)
    {
        this->aotCompiledFunctions = aotCompiledFunctions;
    }

    ~CompilationUnit()
    {
        if (data) {
            if (data->qmlUnit() != qmlData)
                free(const_cast<QmlUnit *>(qmlData));
            qmlData = nullptr;

            if (!(data->flags & QV4::CompiledData::Unit::StaticData))
                free(const_cast<Unit *>(data));
        }
        data = nullptr;
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        delete [] constants;
        constants = nullptr;
#endif

        delete [] imports;
        imports = nullptr;
    }

    CompilationUnit(CompilationUnit &&other) noexcept
    {
        *this = std::move(other);
    }

    CompilationUnit &operator=(CompilationUnit &&other) noexcept
    {
        if (this != &other) {
            data = other.data;
            other.data = nullptr;
            qmlData = other.qmlData;
            other.qmlData = nullptr;
            dynamicStrings = std::move(other.dynamicStrings);
            aotCompiledFunctions = other.aotCompiledFunctions;
            other.dynamicStrings.clear();
            m_fileName = std::move(other.m_fileName);
            other.m_fileName.clear();
            m_finalUrlString = std::move(other.m_finalUrlString);
            other.m_finalUrlString.clear();
            m_module = other.m_module;
            other.m_module = nullptr;
            CompilationUnitBase::operator=(std::move(other));
        }
        return *this;
    }

    const Unit *unitData() const { return data; }

    void setUnitData(const Unit *unitData, const QmlUnit *qmlUnit = nullptr,
                     const QString &fileName = QString(), const QString &finalUrlString = QString())
    {
        data = unitData;
        qmlData = nullptr;
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        delete [] constants;
#endif
        constants = nullptr;
        m_fileName.clear();
        m_finalUrlString.clear();
        if (!data)
            return;

        qmlData = qmlUnit ? qmlUnit : data->qmlUnit();

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        StaticValue *bigEndianConstants = new StaticValue[data->constantTableSize];
        const quint64_le *littleEndianConstants = data->constants();
        for (uint i = 0; i < data->constantTableSize; ++i)
            bigEndianConstants[i] = StaticValue::fromReturnedValue(littleEndianConstants[i]);
        constants = bigEndianConstants;
#else
        constants = reinterpret_cast<const StaticValue*>(data->constants());
#endif

        m_fileName = !fileName.isEmpty() ? fileName : stringAt(data->sourceFileIndex);
        m_finalUrlString = !finalUrlString.isEmpty() ? finalUrlString : stringAt(data->finalUrlIndex);
    }

    QString stringAt(int index) const
    {
        if (uint(index) >= data->stringTableSize)
            return dynamicStrings.at(index - data->stringTableSize);
        return data->stringAtInternal(index);
    }

    QString fileName() const { return m_fileName; }
    QString finalUrlString() const { return m_finalUrlString; }

    Heap::Module *module() const { return m_module; }
    void setModule(Heap::Module *module) { m_module = module; }

    QString bindingValueAsString(const CompiledData::Binding *binding) const
    {
        using namespace CompiledData;
        switch (binding->type()) {
        case Binding::Type_Script:
        case Binding::Type_String:
            return stringAt(binding->stringIndex);
        case Binding::Type_Null:
            return QStringLiteral("null");
        case Binding::Type_Boolean:
            return binding->value.b ? QStringLiteral("true") : QStringLiteral("false");
        case Binding::Type_Number:
            return QString::number(bindingValueAsNumber(binding), 'g', QLocale::FloatingPointShortest);
        case Binding::Type_Invalid:
            return QString();
        case Binding::Type_TranslationById:
        case Binding::Type_Translation:
            return stringAt(data->translations()[binding->value.translationDataIndex].stringIndex);
        default:
            break;
        }
        return QString();
    }

    QString bindingValueAsScriptString(const CompiledData::Binding *binding) const
    {
        return (binding->type() == CompiledData::Binding::Type_String)
                ? CompiledData::Binding::escapedString(stringAt(binding->stringIndex))
                : bindingValueAsString(binding);
    }

    double bindingValueAsNumber(const CompiledData::Binding *binding) const
    {
        if (binding->type() != CompiledData::Binding::Type_Number)
            return 0.0;
        return constants[binding->value.constantValueIndex].doubleValue();
    }

private:
    QString m_fileName; // initialized from data->sourceFileIndex
    QString m_finalUrlString; // initialized from data->finalUrlIndex

    Heap::Module *m_module = nullptr;
};

class SaveableUnitPointer
{
    Q_DISABLE_COPY_MOVE(SaveableUnitPointer)
public:
    SaveableUnitPointer(const Unit *unit, quint32 temporaryFlags = Unit::StaticData) :
          unit(unit),
          temporaryFlags(temporaryFlags)
    {
    }

    ~SaveableUnitPointer() = default;

    template<typename Char>
    bool saveToDisk(const std::function<bool(const Char *, quint32)> &writer) const
    {
        const quint32_le oldFlags = mutableFlags();
        auto cleanup = qScopeGuard([this, oldFlags]() { mutableFlags() = oldFlags; });
        mutableFlags() |= temporaryFlags;
        return writer(data<Char>(), size());
    }

    static bool writeDataToFile(const QString &outputFileName, const char *data, quint32 size,
                                QString *errorString)
    {
#if QT_CONFIG(temporaryfile)
        QSaveFile cacheFile(outputFileName);
        if (!cacheFile.open(QIODevice::WriteOnly | QIODevice::Truncate)
                || cacheFile.write(data, size) != size
                || !cacheFile.commit()) {
            *errorString = cacheFile.errorString();
            return false;
        }

        errorString->clear();
        return true;
#else
        Q_UNUSED(outputFileName);
        *errorString = QStringLiteral("features.temporaryfile is disabled.");
        return false;
#endif
    }

private:
    const Unit *unit;
    quint32 temporaryFlags;

    quint32_le &mutableFlags() const
    {
        return const_cast<Unit *>(unit)->flags;
    }

    template<typename Char>
    const Char *data() const
    {
        Q_STATIC_ASSERT(sizeof(Char) == 1);
        const Char *dataPtr;
        memcpy(&dataPtr, &unit, sizeof(dataPtr));
        return dataPtr;
    }

    quint32 size() const
    {
        return unit->unitSize;
    }
};


} // CompiledData namespace
} // QV4 namespace

Q_DECLARE_TYPEINFO(QV4::CompiledData::JSClassMember, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

#endif
