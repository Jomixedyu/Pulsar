#pragma once
#include "Object.h"
#include "Type.h"
#include "Assembly.h"

#define CORELIB_USE_ATTR(...) static inline struct __corelib_attr_t \
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
        Attribute() {};
        virtual ~Attribute() override {}
    };
    CORELIB_DECL_SHORTSPTR(Attribute);


    class SerializableAttribtue : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::SerializableAttribtue, Attribute);
        CORELIB_USE_ATTR(new Attribute());

    public:

    };
}