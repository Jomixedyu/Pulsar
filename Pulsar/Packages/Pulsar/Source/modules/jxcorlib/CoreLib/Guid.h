#pragma once
#include <cstdint>
#include <string>
#include <string_view>

#ifndef PRE_CORELIB_NO_GUID_TYPE
#include "BasicTypes.h"
#endif

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
        std::string to_string(bool sep = false) const;
        bool is_empty() const;
        bool operator==(guid_t right) const;
        operator bool() const;
        static guid_t empty() { return guid_t(); }
    };

#ifndef PRE_CORELIB_NO_GUID_TYPE
    class Guid : public BoxingObject, public IStringify
    {
        CORELIB_DEF_TYPE(AssemblyObject_jxcorlib, jxcorlib::Guid, BoxingObject);
        CORELIB_IMPL_INTERFACES(IStringify);
    public:
        using unboxing_type = guid_t;
        guid_t get_unboxing_value() const { return this->value_; }

        Guid() : CORELIB_INIT_INTERFACE(IStringify) {}
        Guid(guid_t guid) : value_(guid), CORELIB_INIT_INTERFACE(IStringify) {}
        virtual string ToString() const override { return this->value_.to_string(); }
    public: /*Interfaces*/
        virtual void IStringify_Parse(const string& value) override { this->value_ = guid_t::parse(value); }
        virtual string IStringify_Stringify() override { return this->ToString(); }
    private: /*Fields*/
        guid_t value_;
    };
    CORELIB_DECL_SHORTSPTR(Guid);
#endif
}

#ifndef PRE_CORELIB_NO_GUID_TYPE
CORELIB_DECL_BOXING(jxcorlib::guid_t, jxcorlib::Guid);
#endif

namespace std
{
    template<>
    struct hash<jxcorlib::guid_t>
    {
        size_t operator()(const jxcorlib::guid_t& handle) const noexcept
        {
            constexpr size_t prime = 16777619;
            size_t hash = 2166136261;
            hash = (hash ^ std::hash<uint64_t>()(*reinterpret_cast<const uint64_t*>(&handle))) * prime;
            hash = (hash ^ std::hash<uint64_t>()(*(reinterpret_cast<const uint64_t*>(&handle) + 1))) * prime;
            return hash;

        }
    };
}