#include "PropertyControls/ObjectPropertyControl.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <Pulsar/IconsForkAwesome.h>

namespace pulsared
{
    bool ObjectPropertyControl::OnDrawImGui(const string& name, Object* prop)
    {
        //ImGui::PushItemWidth(-1);

        if (prop)
        {
            Type* type = prop->GetType();
            auto object = static_cast<ObjectBase*>(prop);

            char buf[64];
            auto str = StringUtil::Concat(object->GetName(), " (", object->GetObjectHandle().to_string().substr(24, 8), ")");
            strcpy(buf, str.c_str());
            
            ImGui::InputTextEx("##i", nullptr, buf, 64, ImVec2(-50, 0), 0);

            ImGui::SameLine();
            ImGui::Button(ICON_FK_ARROW_CIRCLE_LEFT, ImVec2(20, 0));
            ImGui::SameLine(0, 3);
            ImGui::Button(ICON_FK_FOLDER_OPEN, ImVec2(20, 0));
        }
        else
        {
            char buf[64] = "Null";

            ImGui::InputTextEx("##Null", nullptr, buf, 64, ImVec2(-50, 0), 0);

            ImGui::SameLine();
            ImGui::Button(ICON_FK_ARROW_CIRCLE_LEFT, ImVec2(20, 0));
            ImGui::SameLine(0, 3);
            ImGui::Button(ICON_FK_FOLDER_OPEN, ImVec2(20, 0));

        }

        //ImGui::PopItemWidth();
        return false;
    }
}