#include "PropertiesPanel/PropertiesNodePanel.h"
#include <PulsarEd/EditorSelection.h>
#include <PulsarEd/PropertyControls/PropertyControl.h>

namespace pulsared
{


    static string _GetComponentDisplayName(Component* com)
    {
        string name = com->GetType()->GetShortName();

        static char com_str[] = "Component";
        if (name.ends_with("Component"))
        {
            name = name.substr(0, name.size() - sizeof(com_str) + 1);
        }
        return StringUtil::FriendlyName(name);
    }
    
    static void _PropertyLine(const string& name, sptr<Object>& obj)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(StringUtil::FriendlyName(name).c_str());
        ImGui::TableSetColumnIndex(1);

        PropertyControlManager::ShowProperty(name, obj);
    }

    static constexpr int kTableRowHeight = 30;

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
        ImGui::PushItemWidth(-1);
        if (ImGui::InputText("##Name", name, 255))
        {
            if (selected->get_name() != name)
            {
                selected->set_name(name);
            }
        }
        ImGui::PopItemWidth();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        //transform info
        if (ImGui::BeginTable("##transform", 2))
        {

            auto boxpos = BoxUtil::Box(selected->get_self_position());
            _PropertyLine("Position", boxpos);
            selected->set_self_position(UnboxUtil::Unbox<Vector3f>(boxpos));

            auto boxrot = BoxUtil::Box(selected->get_self_rotation());
            _PropertyLine("Rotation", boxrot);
            //selected->set_self_rotation(UnboxUtil::Unbox<Quat4f>(boxrot));

            auto boxeuler = BoxUtil::Box(selected->get_self_euler_rotation());
            _PropertyLine("Euler", boxeuler);
            selected->set_self_euler_rotation(UnboxUtil::Unbox<Vector3f>(boxeuler));

            auto boxscale = BoxUtil::Box(selected->get_self_scale());
            _PropertyLine("Scale", boxscale);
            selected->set_self_scale(UnboxUtil::Unbox<Vector3f>(boxscale));

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Components");
        ImGui::SameLine(ImGui::GetWindowWidth() - 180);
        ImGui::Button("Add Component", { 170,30 });

        for (auto& comp : selected->GetAllComponentArray())
        {
            static bool opened = true;
            if (ImGui::CollapsingHeader(_GetComponentDisplayName(comp.get()).c_str(), &opened, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto com_guid = comp->get_object_id().to_string();
                Type* com_type = comp->GetType();
                auto fields = com_type->GetFieldInfos(TypeBinding::NonPublic);
                if (ImGui::BeginTable(com_guid.c_str(), 2))
                {
                    for (auto& field : fields)
                    {
                        //ImGui::TableNextRow(0, kTableRowHeight);
                        //ImGui::TableSetColumnIndex(0);
                        //ImGui::Text(StringUtil::FriendlyName(field->get_name()).c_str());
                        //ImGui::TableSetColumnIndex(1);

                        auto field_inst = field->GetValue(comp.get());
                        _PropertyLine(field->GetName(), field_inst);

                        //auto prop_control = PropertyControlManager::FindControl(field->get_field_type());
                        //if (prop_control)
                        //{
                        //    auto field_inst = field->GetValue(comp.get());
                        //    prop_control->OnDrawImGui(field->get_name(), field_inst);
                        //    field->SetValue(comp.get(), field_inst);
                        //}
                        //else
                        //{
                        //    ImGui::Text("not supported property");
                        //}
                    }

                    //debug info
                    {
                        ImGui::BeginDisabled(true);

                        ImGui::TableNextRow(0, kTableRowHeight);
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Fullname");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text(comp->GetType()->GetName().c_str());

                        ImGui::TableNextRow(0, kTableRowHeight);
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Guid");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text(comp->get_object_id().to_string().c_str());

                        ImGui::EndDisabled();
                    }

                    ImGui::EndTable();
                }
            }
        }

        ImGui::Spacing();

    }
}