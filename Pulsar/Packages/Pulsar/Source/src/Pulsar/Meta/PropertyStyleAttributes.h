#pragma once
#include <CoreLib/Attribute.h>
#include <Pulsar/Assembly.h>

namespace pulsar
{
    class EditStyleAttribute : public jxcorlib::Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::EditStyleAttribute, jxcorlib::Attribute);
    public:
        virtual bool TryParse(const string& style) const { return false; }
        virtual SPtr<EditStyleAttribute> Parse(const string& style) const { return nullptr; }
    };

    class ColorEditAttribute final : public EditStyleAttribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ColorEditAttribute, EditStyleAttribute);
    public:
        bool TryParse(const string& style) const override;
        SPtr<EditStyleAttribute> Parse(const string& style) const override;
    };

    class FloatSliderEditAttribute final : public EditStyleAttribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::FloatSliderEditAttribute, EditStyleAttribute);
    public:
        bool TryParse(const string& style) const override;
        SPtr<EditStyleAttribute> Parse(const string& style) const override;
    };

    class IntegerEditAttribute final : public EditStyleAttribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::IntegerEditAttribute, EditStyleAttribute);
    public:
        bool TryParse(const string& style) const override;
        SPtr<EditStyleAttribute> Parse(const string& style) const override;
    };

    class CheckBoxEditAttribute final : public EditStyleAttribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CheckBoxEditAttribute, EditStyleAttribute);
    public:
        bool TryParse(const string& style) const override;
        SPtr<EditStyleAttribute> Parse(const string& style) const override;
    };

    class IntRangeEditAttribute final : public EditStyleAttribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::IntRangeEditAttribute, EditStyleAttribute);
    public:
        IntRangeEditAttribute() = default;
        IntRangeEditAttribute(float min, float max) : m_min(min), m_max(max) {}

        bool TryParse(const string& style) const override;
        SPtr<EditStyleAttribute> Parse(const string& style) const override;

        float m_min = 0.0f;
        float m_max = 1.0f;
    };

    class FloatRangeEditAttribute final : public EditStyleAttribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::FloatRangeEditAttribute, EditStyleAttribute);
    public:
        FloatRangeEditAttribute() = default;
        FloatRangeEditAttribute(float min, float max) : m_min(min), m_max(max) {}

        bool TryParse(const string& style) const override;
        SPtr<EditStyleAttribute> Parse(const string& style) const override;

        float m_min = 0.0f;
        float m_max = 1.0f;
    };

    class PrecisionEditAttribute final : public EditStyleAttribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::PrecisionEditAttribute, EditStyleAttribute);
    public:
        PrecisionEditAttribute() = default;
        explicit PrecisionEditAttribute(int precision) : m_precision(precision) {}

        bool TryParse(const string& style) const override;
        SPtr<EditStyleAttribute> Parse(const string& style) const override;

        int m_precision = 3;
    };
}
