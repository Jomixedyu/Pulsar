
#include "ShaderConfig.h"

#include <cstdlib>
#include <cstring>

namespace pulsar
{

    static bool TryParseRange(const string& hint, float& min, float& max)
    {
        // Range(0, 1)
        if (hint.starts_with("Range(") && hint.ends_with(")"))
        {
            auto inner = hint.substr(6, hint.size() - 7);
            auto comma = inner.find(',');
            if (comma != string::npos)
            {
                min = static_cast<float>(std::atof(inner.substr(0, comma).c_str()));
                max = static_cast<float>(std::atof(inner.substr(comma + 1).c_str()));
                return true;
            }
        }
        return false;
    }

    static bool TryParseIntRange(const string& hint, int& min, int& max)
    {
        // IntRange(0, 255)
        if (hint.starts_with("IntRange(") && hint.ends_with(")"))
        {
            auto inner = hint.substr(9, hint.size() - 10);
            auto comma = inner.find(',');
            if (comma != string::npos)
            {
                min = std::atoi(inner.substr(0, comma).c_str());
                max = std::atoi(inner.substr(comma + 1).c_str());
                return true;
            }
        }
        return false;
    }

    static bool TryParsePrecision(const string& hint, int& precision)
    {
        // Precision(3)
        if (hint.starts_with("Precision(") && hint.ends_with(")"))
        {
            auto inner = hint.substr(10, hint.size() - 11);
            precision = std::atoi(inner.c_str());
            return true;
        }
        return false;
    }

    bool ShaderPropertyStyle::IsColor() const
    {
        return Hint == "Color";
    }

    bool ShaderPropertyStyle::IsHDR() const
    {
        return Hint == "HDR";
    }

    bool ShaderPropertyStyle::IsSlider() const
    {
        return Hint == "Slider";
    }

    bool ShaderPropertyStyle::IsInteger() const
    {
        return Hint == "Integer";
    }

    bool ShaderPropertyStyle::IsIntRange(int& min, int& max) const
    {
        return TryParseIntRange(Hint, min, max);
    }

    bool ShaderPropertyStyle::IsRange(float& min, float& max) const
    {
        return TryParseRange(Hint, min, max);
    }

    bool ShaderPropertyStyle::IsNormal() const
    {
        return Hint == "Normal";
    }

    bool ShaderPropertyStyle::IsVector() const
    {
        return Hint == "Vector";
    }

    bool ShaderPropertyStyle::GetPrecision(int& precision) const
    {
        return TryParsePrecision(Hint, precision);
    }

}
