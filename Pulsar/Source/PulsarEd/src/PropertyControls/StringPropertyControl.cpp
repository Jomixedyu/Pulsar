#include "PropertyControls/StringPropertyControl.h"
#include <imgui/imgui.h>

namespace pulsared
{
    static char tempString[1024];
    bool StringPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop)
    {
        assert(prop && prop->GetType() == GetPropertyType());

        auto value = static_cast<String*>(prop);
        ImGui::PushItemWidth(-1);

        StringUtil::strcpy(tempString, *value);
        ImGui::PushID(name.c_str());
        bool changed = ImGui::InputText("input", tempString, sizeof(tempString));
        if (changed)
        {
            *value = tempString;
        }
        ImGui::PopID();
        ImGui::PopItemWidth();
        return changed;
    }
}