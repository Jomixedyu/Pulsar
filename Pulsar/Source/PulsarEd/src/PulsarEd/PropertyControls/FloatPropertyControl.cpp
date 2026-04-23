#include "PropertyControls/FloatPropertyControl.h"
#include <CoreLib/Attribute.h>
#include <Pulsar/Meta/PropertyStyleAttributes.h>
#include <imgui/imgui.h>

namespace pulsared
{
    bool FloatPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop, std::span<Attribute*> attrs)
    {
        assert(prop && prop->GetType() == GetPropertyType());

        auto f = static_cast<Single32*>(prop);

        // check for PrecisionEditAttribute
        const char* format = "%.3f";
        for (auto* attr : attrs)
        {
            if (attr->GetType() == cltypeof<pulsar::PrecisionEditAttribute>() || attr->GetType()->IsSubclassOf(cltypeof<pulsar::PrecisionEditAttribute>()))
            {
                auto* precAttr = static_cast<pulsar::PrecisionEditAttribute*>(attr);
                static char formatBuf[16];
                snprintf(formatBuf, sizeof(formatBuf), "%%.%df", precAttr->m_precision);
                format = formatBuf;
                break;
            }
        }

        // check for RangeEditAttribute and Slider/Integer hints via custom attributes
        float rangeMin = 0.f, rangeMax = 0.f;
        bool hasRange = false;
        bool isSlider = false;
        bool isInteger = false;
        bool isCheckBox = false;
        for (auto* attr : attrs)
        {
            if (attr->GetType() == cltypeof<pulsar::FloatRangeEditAttribute>() || attr->GetType()->IsSubclassOf(cltypeof<pulsar::FloatRangeEditAttribute>()))
            {
                auto* rangeAttr = static_cast<pulsar::FloatRangeEditAttribute*>(attr);
                rangeMin = rangeAttr->m_min;
                rangeMax = rangeAttr->m_max;
                hasRange = true;
            }
            else if (attr->GetType() == cltypeof<pulsar::FloatSliderEditAttribute>() || attr->GetType()->IsSubclassOf(cltypeof<pulsar::FloatSliderEditAttribute>()))
            {
                isSlider = true;
            }
            else if (attr->GetType() == cltypeof<pulsar::IntegerEditAttribute>() || attr->GetType()->IsSubclassOf(cltypeof<pulsar::IntegerEditAttribute>()))
            {
                isInteger = true;
            }
            else if (attr->GetType() == cltypeof<pulsar::IntRangeEditAttribute>() || attr->GetType()->IsSubclassOf(cltypeof<pulsar::IntRangeEditAttribute>()))
            {
                auto* intRange = static_cast<pulsar::IntRangeEditAttribute*>(attr);
                isInteger = true;
                rangeMin = intRange->m_min;
                rangeMax = intRange->m_max;
                hasRange = true;
            }
            else if (attr->GetType() == cltypeof<pulsar::CheckBoxEditAttribute>() || attr->GetType()->IsSubclassOf(cltypeof<pulsar::CheckBoxEditAttribute>()))
            {
                isCheckBox = true;
            }
        }

        ImGui::PushItemWidth(-1);
        bool changed = false;
        if (isCheckBox)
        {
            bool bv = f->value != 0.0f;
            changed = ImGui::Checkbox(("##" + name).c_str(), &bv);
            if (changed) f->value = bv ? 1.0f : 0.0f;
        }
        else if (isSlider && hasRange)
        {
            changed = ImGui::SliderFloat(("##" + name).c_str(), &f->value, rangeMin, rangeMax, format);
        }
        else if (isInteger)
        {
            int iv = static_cast<int>(f->value);
            changed = ImGui::DragInt(("##" + name).c_str(), &iv, 1.0f,
                hasRange ? static_cast<int>(rangeMin) : 0,
                hasRange ? static_cast<int>(rangeMax) : 0);
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
