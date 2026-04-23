#include "PropertyControls/Vector4fPropertyControl.h"
#include <imgui/imgui.h>

namespace pulsared
{
    bool Vector4fPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop, std::span<Attribute*> attrs)
    {
        assert(prop && prop->GetType() == GetPropertyType());
        auto f = static_cast<pulsar::math::BoxingVector4f*>(prop);
        ImGui::PushItemWidth(-1);
        float f4[] = { f->x, f->y, f->z, f->w };
        bool changed = ImGui::DragFloat4(("##" + name).c_str(), f4, 0.002f, 0.f, 0.f, "%.3f");
        if (changed)
        {
            f->x = f4[0];
            f->y = f4[1];
            f->z = f4[2];
            f->w = f4[3];
        }
        ImGui::PopItemWidth();
        return changed;
    }
}
