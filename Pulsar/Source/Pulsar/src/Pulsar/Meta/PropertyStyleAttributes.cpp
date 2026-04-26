#include "PropertyStyleAttributes.h"
#include <cstdlib>

namespace pulsar
{
    bool ColorEditAttribute::TryParse(const string& style) const
    {
        return style == "Color";
    }

    SPtr<EditStyleAttribute> ColorEditAttribute::Parse(const string&) const
    {
        return mksptr(new ColorEditAttribute());
    }

    bool FloatSliderEditAttribute::TryParse(const string& style) const
    {
        return style == "FloatSlider";
    }

    SPtr<EditStyleAttribute> FloatSliderEditAttribute::Parse(const string&) const
    {
        return mksptr(new FloatSliderEditAttribute());
    }

    bool IntegerEditAttribute::TryParse(const string& style) const
    {
        return style == "Integer";
    }

    SPtr<EditStyleAttribute> IntegerEditAttribute::Parse(const string&) const
    {
        return mksptr(new IntegerEditAttribute());
    }

    bool CheckBoxEditAttribute::TryParse(const string& style) const
    {
        return style == "CheckBox";
    }

    SPtr<EditStyleAttribute> CheckBoxEditAttribute::Parse(const string&) const
    {
        return mksptr(new CheckBoxEditAttribute());
    }

    bool IntRangeEditAttribute::TryParse(const string& style) const
    {
        return style == "IntRange" ||
               (style.starts_with("IntRange(") && style.ends_with(")"));
    }

    SPtr<EditStyleAttribute> IntRangeEditAttribute::Parse(const string& style) const
    {
        size_t openParen = style.find('(');
        if (openParen != string::npos && style.ends_with(")"))
        {
            auto inner = style.substr(openParen + 1, style.size() - openParen - 2);
            auto comma = inner.find(',');
            if (comma != string::npos)
            {
                float min = static_cast<float>(std::atof(inner.substr(0, comma).c_str()));
                float max = static_cast<float>(std::atof(inner.substr(comma + 1).c_str()));
                return mksptr(new IntRangeEditAttribute(min, max));
            }
        }
        return mksptr(new IntRangeEditAttribute());
    }

    bool FloatRangeEditAttribute::TryParse(const string& style) const
    {
        return style.starts_with("FloatRange(") && style.ends_with(")");
    }

    SPtr<EditStyleAttribute> FloatRangeEditAttribute::Parse(const string& style) const
    {
        auto inner = style.substr(11, style.size() - 12);
        auto comma = inner.find(',');
        if (comma == string::npos)
            return mksptr(new FloatRangeEditAttribute());

        float min = static_cast<float>(std::atof(inner.substr(0, comma).c_str()));
        float max = static_cast<float>(std::atof(inner.substr(comma + 1).c_str()));
        return mksptr(new FloatRangeEditAttribute(min, max));
    }

    bool PrecisionEditAttribute::TryParse(const string& style) const
    {
        return style.starts_with("Precision(") && style.ends_with(")");
    }

    SPtr<EditStyleAttribute> PrecisionEditAttribute::Parse(const string& style) const
    {
        auto inner = style.substr(10, style.size() - 11);
        int prec = std::atoi(inner.c_str());
        return mksptr(new PrecisionEditAttribute(prec));
    }
}
