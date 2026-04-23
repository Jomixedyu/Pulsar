#include "PropertyControls/FloatPropertyControl.h"
#include <CoreLib/Attribute.h>
#include <imgui/imgui.h>

namespace pulsared
{
    bool FloatPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop, std::span<Attribute*> attrs)
    {
        assert(prop && prop->GetType() == GetPropertyType());

        auto f = static_cast<Single32*>(prop);

        // check for PrecisionAttribute
        const char* format = "%.3f";
        for (auto* attr : attrs)
        {
            if (attr->GetType() == cltypeof<PrecisionAttribute>() || attr->GetType()->IsSubclassOf(cltypeof<PrecisionAttribute>()))
            {
                auto* precAttr = static_cast<PrecisionAttribute*>(attr);
                static char formatBuf[16];
                snprintf(formatBuf, sizeof(formatBuf), "%%.%df", precAttr->GetPrecision());
                format = formatBuf;
                break;
            }
        }

        // check for RangeAttribute and Slider/Integer hints via custom attributes
        float rangeMin = 0.f, rangeMax = 0.f;
        bool hasRange = false;
        bool isSlider = false;
        bool isInteger = false;
        for (auto* attr : attrs)
        {
            if (attr->GetType() == cltypeof<RangePropertyAttribute>() || attr->GetType()->IsSubclassOf(cltypeof<RangePropertyAttribute>()))
            {
                auto* rangeAttr = static_cast<RangePropertyAttribute*>(attr);
                rangeMin = rangeAttr->m_min;
                rangeMax = rangeAttr->m_max;
                hasRange = true;
            }
            else if (attr->GetType() == cltypeof<SliderPropertyAttribute>() || attr->GetType()->IsSubclassOf(cltypeof<SliderPropertyAttribute>()))
            {
                isSlider = true;
            }
            else if (attr->GetType() == cltypeof<IntegerEditAttribute>() || attr->GetType()->IsSubclassOf(cltypeof<IntegerEditAttribute>()))
            {
                isInteger = true;
            }
        }

        ImGui::PushItemWidth(-1);
        bool changed = false;
        if (isSlider && hasRange)
        {
            changed = ImGui::SliderFloat(("##" + name).c_str(), &f->value, rangeMin, rangeMax, format);
        }
        else if (isInteger)
        {
            int iv = static_cast<int>(f->value);
            changed = ImGui::DragInt(("##" + name).c_str(), &iv);
            if (changed) f->value = static_cast<float>(iv);
        }
        else
        {
            changed = ImGui::DragFloat(("##" + name).c_str(), &f->value, 0.002f,
                hasRange ? rangeMin : 0.f,
                hasRange ? rangeMax : 0.f,
                format);
        }
        ImGui::PopItemWidth();
        return changed;
    }
}
