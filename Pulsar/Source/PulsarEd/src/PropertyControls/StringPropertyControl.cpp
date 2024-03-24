#include "PropertyControls/StringPropertyControl.h"
#include <imgui/imgui.h>

namespace pulsared
{
    bool StringPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop)
    {
        assert(prop && prop->GetType() == GetPropertyType());

        auto value = static_cast<String*>(prop);
        ImGui::PushItemWidth(-1);
        char buf[1024]{};
        StringUtil::strcpy(buf, value->c_str());
        ImGui::PushID(prop);
        bool changed = ImGui::InputText("input", buf, 1024);
        ImGui::PopID();
        *value = buf;
        ImGui::PopItemWidth();
        return changed;
    }
}