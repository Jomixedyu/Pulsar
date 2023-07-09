#include "PropertyControls/MaterialPropertyControl.h"
#include <ThirdParty/imgui/imgui.h>

namespace pulsared
{
    void MaterialPropertyControl::OnDrawImGui(const string& name, sptr<Object> prop)
    {
        assert(prop && prop->GetType() == GetPropertyType());
        Type* type = prop->GetType();
        auto material = sptr_cast<Material>(prop);

        ImGui::PushItemWidth(-1);
        
        ImGui::Text(material->get_name().c_str());

        ImGui::PopItemWidth();
    }
}