#include "PropertyControls/Quat4fPropertyControl.h"
#include <imgui/imgui.h>

namespace pulsared
{
    bool Quat4fPropertyControl::OnDrawImGui(const string& name, Object* prop)
    {
        assert(prop && prop->GetType() == GetPropertyType());
        Type* type = prop->GetType();
        auto q = static_cast<pulsar::math::BoxingQuat4f*>(prop);
        ImGui::PushItemWidth(-1);

        float f[] = { q->x, q->y, q->z, q->w };
        bool changed = ImGui::DragFloat4(("##" + name).c_str(), f, 0.2f, 0.f, 0.f, "%.3f");
        if (changed)
        {
            q->x = f[0];
            q->y = f[1];
            q->z = f[2];
            q->w = f[3];
        }

        ImGui::PopItemWidth();

        return changed;
    }
}