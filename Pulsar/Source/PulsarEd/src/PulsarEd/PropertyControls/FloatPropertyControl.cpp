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

        ImGui::PushItemWidth(-1);
        bool changed = ImGui::DragFloat(("##" + name).c_str(), &f->value, 0.002f, 0.f, 0.f, format);
        ImGui::PopItemWidth();
        return changed;
    }
}
