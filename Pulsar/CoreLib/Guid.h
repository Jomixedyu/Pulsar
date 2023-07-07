#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include "BasicTypes.h"

namespace jxcorlib
{
    struct alignas(16) guid_t
    {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t z = 0;
        uint32_t w = 0;
    public:
        
    public:
        static guid_t create_new();
        static guid_t parse(std::string_view str);
    public:
        std::string to_string() const;
        bool is_empty() const;
        bool operator==(guid_t right) const;
        operator bool() const;
        static guid_t empty() { return guid_t(); }
    };

    class Guid : public BoxingObject, public IStringify
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::Guid, BoxingObject);
        CORELIB_IMPL_INTERFACES(IStringify);
    public:
        using unboxing_type = guid_t;
        guid_t get_unboxing_value() const { return this->value_; }

        Guid() :CORELIB_INIT_INTERFACE(IStringify) {}
        Guid(guid_t guid) : value_(guid), CORELIB_INIT_INTERFACE(IStringify) {}
        virtual string ToString() const override { return this->value_.to_string(); }
    public: /*Interfaces*/
        virtual void IStringify_Parse(const string& value) override { this->value_ = guid_t::parse(value); }
        virtual string IStringify_Stringify() override { return this->ToString(); }
    private: /*Fields*/
        guid_t value_;
    };
    CORELIB_DECL_SHORTSPTR(Guid);

    template<> struct get_boxing_type<guid_t> { using type = Guid; };

}