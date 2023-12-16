#include "PropertyControls/FloatPropertyControl.h"
#include <imgui/imgui.h>

namespace pulsared
{
    bool FloatPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop)
    {
        assert(prop && prop->GetType() == GetPropertyType());

        auto f = static_cast<Single32*>(prop);
        ImGui::PushItemWidth(-1);
        bool changed = ImGui::DragFloat(("##" + name).c_str(), &f->value, 0.002f);
        ImGui::PopItemWidth();
        return changed;
    }
}