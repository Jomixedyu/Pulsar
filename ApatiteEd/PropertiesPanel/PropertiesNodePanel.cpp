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
        if (ImGui::InputText("Name", name, 255))
        {
            if (selected->get_name() != name)
            {
                selected->set_name(name);
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        auto position = selected->get_self_position();
        ImGui::InputFloat3("Position", position.get_value_ptr());
        selected->set_self_position(position);

        auto rotation = selected->get_self_euler_rotation();
        ImGui::InputFloat3("Rotation", rotation.get_value_ptr());
        selected->set_self_euler_rotation(rotation);

        auto scale = selected->get_self_scale();
        ImGui::InputFloat3("scale", scale.get_value_ptr());
        selected->set_self_scale(scale);


        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        for (auto& comp : selected->GetAllComponentArray())
        {
            static bool opened = true;
            if (ImGui::CollapsingHeader(comp->GetType()->get_name().c_str(), &opened, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
            {

            }
        }

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Button("Add Component", { 300,30 });
    }
}