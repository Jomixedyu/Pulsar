#include "PropertyStyleAttributes.h"
#include <cstdlib>

namespace pulsar
{
    bool ColorEditAttribute::TryParse(const string& style) const
    {
        return style == "Color";
    }

    std::vector<SPtr<jxcorlib::Attribute>> ColorEditAttribute::Parse(const string&) const
    {
        return { mksptr(new ColorEditAttribute()) };
    }

    bool FloatSliderEditAttribute::TryParse(const string& style) const
    {
        return style == "FloatSlider";
    }

    std::vector<SPtr<jxcorlib::Attribute>> FloatSliderEditAttribute::Parse(const string&) const
    {
        return { mksptr(new FloatSliderEditAttribute()) };
    }

    bool IntegerEditAttribute::TryParse(const string& style) const
    {
        return style == "Integer";
    }

    std::vector<SPtr<jxcorlib::Attribute>> IntegerEditAttribute::Parse(const string&) const
    {
        return { mksptr(new IntegerEditAttribute()) };
    }

    bool IntRangeEditAttribute::TryParse(const string& style) const
    {
        return style == "IntRange" ||
               (style.starts_with("IntRange(") && style.ends_with(")"));
    }

    std::vector<SPtr<jxcorlib::Attribute>> IntRangeEditAttribute::Parse(const string& style) const
    {
        std::vector<SPtr<jxcorlib::Attribute>> result;
        result.push_back(mksptr(new IntegerEditAttribute()));

        size_t openParen = style.find('(');
        if (openParen != string::npos && style.ends_with(")"))
        {
            auto inner = style.substr(openParen + 1, style.size() - openParen - 2);
            auto comma = inner.find(',');
            if (comma != string::npos)
            {
                float min = static_cast<float>(std::atof(inner.substr(0, comma).c_str()));
                float max = static_cast<float>(std::atof(inner.substr(comma + 1).c_str()));
                result.push_back(mksptr(new FloatRangeEditAttribute(min, max)));
            }
        }
        return result;
    }

    bool FloatRangeEditAttribute::TryParse(const string& style) const
    {
        return style.starts_with("FloatRange(") && style.ends_with(")");
    }

    std::vector<SPtr<jxcorlib::Attribute>> FloatRangeEditAttribute::Parse(const string& style) const
    {
        auto inner = style.substr(11, style.size() - 12);
        auto comma = inner.find(',');
        if (comma == string::npos)
            return {};

        float min = static_cast<float>(std::atof(inner.substr(0, comma).c_str()));
        float max = static_cast<float>(std::atof(inner.substr(comma + 1).c_str()));
        return { mksptr(new FloatRangeEditAttribute(min, max)) };
    }

    bool PrecisionEditAttribute::TryParse(const string& style) const
    {
        return style.starts_with("Precision(") && style.ends_with(")");
    }

    std::vector<SPtr<jxcorlib::Attribute>> PrecisionEditAttribute::Parse(const string& style) const
    {
        auto inner = style.substr(10, style.size() - 11);
        int prec = std::atoi(inner.c_str());
        return { mksptr(new PrecisionEditAttribute(prec)) };
    }
}
