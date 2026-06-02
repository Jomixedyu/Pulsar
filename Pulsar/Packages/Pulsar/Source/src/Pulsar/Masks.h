#pragma once
#include "Assembly.h"

namespace pulsar
{
    struct Mask8
    {
        uint8_t Value{};

        Mask8() = default;
        Mask8(uint8_t value) : Value(value) {}

        static Mask8 parse(std::string_view str)
        {
            Mask8 result{};
            int base = 0; // auto-detect: 0x→16, 0→8, otherwise 10
            const char* begin = str.data();
            if (str.starts_with("0b") || str.starts_with("0B"))
            {
                begin += 2;
                base = 2;
            }
            result.Value = static_cast<uint8_t>(std::strtoul(begin, nullptr, base));
            return result;
        }

        std::string to_string() const
        {
            return std::to_string(Value);
        }
    };

    class BoxingMask8 : public BoxingObject, public IStringify
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BoxingMask8, Object);
        CORELIB_IMPL_INTERFACES(IStringify);
    public:
        using unboxing_type = Mask8;
        auto get_unboxing_value() { return m_mask; }

        BoxingMask8() : CORELIB_INIT_INTERFACE(IStringify) {}
        BoxingMask8(Mask8 value) : m_mask(value), CORELIB_INIT_INTERFACE(IStringify) {}

        void IStringify_Parse(const string& value) override
        {
            m_mask = Mask8::parse(value);
        }
        string IStringify_Stringify() override
        {
            return m_mask.to_string();
        }

        string ToString() const override
        {
            return m_mask.to_string();
        }

        uint8_t GetValue() const { return m_mask.Value; }
        void SetValue(uint8_t value) { m_mask.Value = value; }

        Mask8 m_mask{};
    };
}
CORELIB_DECL_BOXING(pulsar::Mask8, pulsar::BoxingMask8);
