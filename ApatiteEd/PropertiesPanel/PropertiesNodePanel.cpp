#include "PropertiesNodePanel.h"
#include <ApatiteEd/EditorSelection.h>

namespace apatiteed
{
    static string _GetComponentDisplayName(Component* com)
    {
        string name = com->GetType()->get_short_name();
        
        static char com_str[] = "Component";
        if (name.ends_with("Component"))
        {
            name = name.substr(0, name.size() - sizeof(com_str) + 1);
        }
        return name;
    }
    void PropertiesNodePanel::OnDrawImGui()
    {
        Node_sp selected = sptr_cast<Node>(EditorSelection::Selection.GetSelected());

        char name[255];
        strcpy_s(name, 255, selected->get_name().c_str());
        
        bool is_active = selected->get_is_active_self();
        ImGui::Checkbox("##active", &is_active);
        if (is_active != selected->get_is_active_self())
        {
            selected->set_is_active_self(is_active);
        }
        ImGui::SameLine();
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
            if (ImGui::CollapsingHeader(_GetComponentDisplayName(comp.get()).c_str(), &opened, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
            {
                //debug
                //ImGui::Text("fullname: %s", comp->GetType()->get_name().c_str());
                //ImGui::Text("guid: %s", comp->get_object_id().to_string().c_str());
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Button("Add Component", { 300,30 });
    }
}