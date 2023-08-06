#pragma once
#include "Object.h"
#include "Type.h"
#include "BasicTypes.h"
#include "Assembly.h"

#define CORELIB_DEF_BOXING_ENUM(ASSEMBLY, NAMESPACE, NAME, DEFS) \
class Boxing##NAME final : public ::jxcorlib::Enum \
{ \
    CORELIB_DEF_ENUMTYPE(ASSEMBLY, NAMESPACE::Boxing##NAME, ::jxcorlib::Enum); \
    static inline DataMap* definitions = nullptr; \
    static inline const char* string_definitions = DEFS; \
    static void init_defs() { InitDefinitions(string_definitions, &definitions); } \
public: \
    using unboxing_type = NAME; \
    static const DataMap* StaticGetDefinitions() { \
        init_defs(); \
        return definitions; \
    } \
    static ::jxcorlib::string StaticFindName(NAME value) { \
        init_defs(); \
        for (auto& [name, enum_value] : *definitions) { \
            if (enum_value == static_cast<uint32_t>(value)) return name; \
        } \
        return {}; \
    } \
    NAME get_unboxing_value() const { return static_cast<NAME>(this->m_value); } \
    ::jxcorlib::string GetName() const override { return StaticFindName(static_cast<NAME>(this->m_value)); } \
    Boxing##NAME& operator=(NAME value) { this->m_value = static_cast<uint32_t>(value); return *this; } \
    Boxing##NAME(NAME value) : base(static_cast<uint32_t>(value)) { } \
    Boxing##NAME() : base() {  } \
    virtual ::jxcorlib::string ToString() const override { return this->GetName(); } \
}; \
CORELIB_DECL_SHORTSPTR(Boxing##NAME);

#define CORELIB_DEF_ENUM(ASSEMBLY, NAMESPACE, NAME, ...) \
enum class NAME : uint32_t \
{ \
 __VA_ARGS__ \
}; \
CORELIB_DEF_BOXING_ENUM(ASSEMBLY, NAMESPACE, NAME, #__VA_ARGS__)

#define CORELIB_DECL_BOXING_ENUM(Enum) CORELIB_DECL_BOXING(Enum, Boxing##Enum);

namespace jxcorlib
{
    class Enum : public BoxingObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::Enum, BoxingObject);
    public:
        using DataMap = Type::EnumDatas;
    protected:
        static void InitDefinitions(const char* datas, DataMap** defs);
    public:
        Enum() : m_value(0) {}
        Enum(uint32_t value) : m_value(value) {}

        uint32_t GetValue() { return this->m_value; }
        void SetValue(uint32_t value) { this->m_value = value; }

        virtual string GetName() const = 0;
        
        uint32_t get_unboxing_value() const { return this->m_value; }

        static bool StaticTryParse(Type* type, string_view name, uint32_t* out_value);

    protected:
        uint32_t m_value;
    };
    CORELIB_DECL_SHORTSPTR(Enum);

}