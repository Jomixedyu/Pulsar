#include "PropertiesNodePanel.h"

namespace apatiteed
{

    void PropertiesNodePanel::OnDrawImGui()
    {
        static char namebuf[255] = "GameObject 1";
        ImGui::InputText("name", namebuf, 255);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        static float position[3];
        static float rotation[3];
        static float scale[3];
        static bool opened = true;
        if (ImGui::CollapsingHeader("Transform", &opened, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::InputFloat3("position", position);
            ImGui::InputFloat3("rotation", rotation);
            ImGui::InputFloat3("scale", scale);
        }
        if (ImGui::CollapsingHeader("MeshFilter", &opened, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
        {
            static char meshpath[255] = "data/a.fbx";
            ImGui::InputText("mesh", meshpath, 255);
        }
        if (ImGui::CollapsingHeader("MeshRenderer", &opened, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
        {
            static char meshpath[255] = "self:MeshFilter";
            ImGui::InputText("meshfilter", meshpath, 255);
        }

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Button("Add Component", { 200,30 });
    }
}