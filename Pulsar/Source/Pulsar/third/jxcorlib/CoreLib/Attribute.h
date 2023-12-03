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


    class SerializableAttribtue : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::SerializableAttribtue, Attribute);
    public:

    };

    class ListItemAttribute final : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::SerializableAttribtue, Attribute);
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
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::HidePropertyAttribute, Attribute);
    public:

    };
}