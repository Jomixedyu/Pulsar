#include "PropertyControls/Vector2fPropertyControl.h"
#include <imgui/imgui.h>

namespace pulsared
{
    bool Vector2fPropertyControl::OnDrawImGui(const string& name, Object* prop)
    {
        assert(prop && prop->GetType() == GetPropertyType());
        Type* type = prop->GetType();
        auto f = static_cast<pulsar::math::BoxingVector2f*>(prop);
        ImGui::PushItemWidth(-1);
        float f3[] = { f->x, f->y };
        bool changed = ImGui::DragFloat2(("##" + name).c_str(), f3, 0.2f);
        if (changed)
        {
            f->x = f3[0];
            f->y = f3[1];
        }
        ImGui::PopItemWidth();
        return changed;
    }
}