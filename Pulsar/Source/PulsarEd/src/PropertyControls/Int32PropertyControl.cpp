#include "PropertyControls/Int32PropertyControl.h"
#include <imgui/imgui.h>

namespace pulsared
{
    bool Int32PropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop)
    {
        assert(prop && prop->GetType() == GetPropertyType());

        auto f = static_cast<Integer32*>(prop);
        ImGui::PushItemWidth(-1);
        bool changed = ImGui::DragInt(("##" + name).c_str(), &f->value, 0.002f);
        ImGui::PopItemWidth();
        return changed;
    }
}