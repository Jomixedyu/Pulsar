#include "PropertyControls/Vector4fPropertyControl.h"
#include <Pulsar/Meta/PropertyStyleAttributes.h>
#include <imgui/imgui.h>

namespace pulsared
{
    bool Vector4fPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop, std::span<Attribute*> attrs)
    {
        assert(prop && prop->GetType() == GetPropertyType());
        auto f = static_cast<pulsar::math::BoxingVector4f*>(prop);
        ImGui::PushItemWidth(-1);

        bool isColor = false;
        for (auto* attr : attrs)
        {
            if (attr->GetType() == cltypeof<pulsar::ColorEditAttribute>() ||
                attr->GetType()->IsSubclassOf(cltypeof<pulsar::ColorEditAttribute>()))
            {
                isColor = true;
                break;
            }
        }

        bool changed = false;
        if (isColor)
        {
            float color[4] = { f->x, f->y, f->z, f->w };
            changed = ImGui::ColorEdit4(("##" + name).c_str(), color);
            if (changed)
            {
                f->x = color[0];
                f->y = color[1];
                f->z = color[2];
                f->w = color[3];
            }
        }
        else
        {
            float f4[] = { f->x, f->y, f->z, f->w };
            changed = ImGui::DragFloat4(("##" + name).c_str(), f4, 0.002f, 0.f, 0.f, "%.3f");
            if (changed)
            {
                f->x = f4[0];
                f->y = f4[1];
                f->z = f4[2];
                f->w = f4[3];
            }
        }
        ImGui::PopItemWidth();
        return changed;
    }
}
