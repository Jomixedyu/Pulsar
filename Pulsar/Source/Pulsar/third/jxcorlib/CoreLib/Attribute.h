#pragma once
#include "Object.h"
#include "Type.h"
#include "Assembly.h"

#define CORELIB_CLASS_ATTR(...) static inline struct __corelib_attr_t \
{ \
    __corelib_attr_t() { \
         ::jxcorlib::TypeBuilder::RegisterAttributes(StaticType(), __VA_ARGS__);\
    } \
} __corelib_attr_;


namespace jxcorlib
{
    class Attribute : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::Attribute, Object);
    public:
        Attribute() = default;
        ~Attribute() override = default;
    };
    CORELIB_DECL_SHORTSPTR(Attribute);

    class NoSerializableAttribtue : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::NoSerializableAttribtue, Attribute);
    public:

    };

    class ListItemAttribute final : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::ListItemAttribute, Attribute);
    public:
        explicit ListItemAttribute(Type* itemType) : m_itemType(itemType) {}

        Type* GetItemType() const { return m_itemType; }
    private:
        Type* m_itemType;
    };

    class HidePropertyAttribute final : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::HidePropertyAttribute, Attribute);
    public:

    };

    class ReadOnlyPropertyAttribute final : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::ReadOnlyPropertyAttribute, Attribute);
    public:

    };

    class RangePropertyAttribute final : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::RangePropertyAttribute, Attribute);
    public:
        RangePropertyAttribute(float min, float max) : m_min(min), m_max(max)
        {
        }

        float m_min;
        float m_max;
    };

    class DebugPropertyAttribute final : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::DebugPropertyAttribute, Attribute);
    public:

    };

}