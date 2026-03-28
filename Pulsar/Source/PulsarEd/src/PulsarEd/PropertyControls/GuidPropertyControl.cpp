#include "PropertyControls/GuidPropertyControl.h"
#include <imgui/imgui.h>

namespace pulsared
{
    static char tempString[1024];
    bool GuidPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop)
    {
        assert(prop && prop->GetType() == GetPropertyType());

        auto value = static_cast<Guid*>(prop);
        ImGui::PushItemWidth(-1);

        ImGui::Text(value->ToString().c_str());
        ImGui::PopItemWidth();

        return false;
    }
}