#include "PropertyControls/Vector3fPropertyControl.h"
#include <imgui/imgui.h>

namespace pulsared
{
    bool Vector3fPropertyControl::OnDrawImGui(const string& name, Object* prop)
    {
        assert(prop && prop->GetType() == GetPropertyType());
        Type* type = prop->GetType();
        auto f = static_cast<pulsar::math::BoxingVector3f*>(prop);
        ImGui::PushItemWidth(-1);
        float f3[] = { f->x, f->y, f->z };
        bool changed = ImGui::DragFloat3(("##" + name).c_str(), f3, 0.2f, 0.f, 0.f, "%.3f");
        if (changed)
        {
            f->x = f3[0];
            f->y = f3[1];
            f->z = f3[2];
        }
        ImGui::PopItemWidth();
        return changed;
    }
}