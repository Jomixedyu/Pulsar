#include "PropertiesNodePanel.h"
#include <ApatiteEd/EditorSelection.h>

namespace apatiteed
{

    void PropertiesNodePanel::OnDrawImGui()
    {
        static char namebuf[255] = "GameObject 1";

        Node_sp selected = sptr_cast<Node>(EditorSelection::Selection.GetSelected());

        char name[255];
        strcpy_s(name, 255, selected->get_name().c_str());
        if (ImGui::InputText("name", name, 255))
        {
            if (selected->get_name() != name)
            {
                selected->set_name(name);
            }
        }

        auto position = selected->get_self_position();
        ImGui::InputFloat3("position", position.get_value_ptr());
        selected->set_self_position(position);

        auto rotation = selected->get_self_euler_rotation();
        ImGui::InputFloat3("rotation", rotation.get_value_ptr());
        selected->set_self_euler_rotation(rotation);

        auto scale = selected->get_self_scale();
        ImGui::InputFloat3("scale", scale.get_value_ptr());
        selected->set_self_scale(scale);

        //ImGui::InputFloat3("rotation", rotation);
        //ImGui::InputFloat3("scale", scale);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        //static float position[3];
        //static float rotation[3];
        //static float scale[3];
        //static bool opened = true;
        //if (ImGui::CollapsingHeader("Transform", &opened, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
        //{
        //    ImGui::InputFloat3("position", position);
        //    ImGui::InputFloat3("rotation", rotation);
        //    ImGui::InputFloat3("scale", scale);
        //}
        //if (ImGui::CollapsingHeader("MeshFilter", &opened, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
        //{
        //    static char meshpath[255] = "data/a.fbx";
        //    ImGui::InputText("mesh", meshpath, 255);
        //}
        //if (ImGui::CollapsingHeader("MeshRenderer", &opened, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
        //{
        //    static char meshpath[255] = "self:MeshFilter";
        //    ImGui::InputText("meshfilter", meshpath, 255);
        //}

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Button("Add Component", { 200,30 });
    }
}