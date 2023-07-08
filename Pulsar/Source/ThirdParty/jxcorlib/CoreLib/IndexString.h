#pragma once

#include "index_string.hpp"
#include "Object.h"
#include "Type.h"
#include "BasicTypes.h"

namespace jxcorlib
{

    class IndexString : public Object, public IStringify
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::IndexString, Object);
        CORELIB_IMPL_INTERFACES(IStringify);
    public:

        IndexString() : IndexString(index_string{}) {}

        IndexString(index_string index) :
            index(index),
            CORELIB_INIT_INTERFACE(IStringify)
        {
        }

    public:
        virtual string ToString() const override { return this->index.to_string(); }
        index_string get_unboxing_value() { return this->index; }

        virtual void IStringify_Parse(const string& value)
        {
            this->index = value.c_str();
        }
        virtual string IStringify_Stringify()
        {
            return this->index.to_string();
        }

    protected:
        index_string index;
    };

    CORELIB_DECL_SHORTSPTR(IndexString);
    template<> struct get_boxing_type<index_string> { using type = IndexString; };

}
