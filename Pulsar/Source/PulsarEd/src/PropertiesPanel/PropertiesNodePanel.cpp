#include "PropertiesPanel/PropertiesNodePanel.h"
#include <PulsarEd/EditorSelection.h>
#include <PulsarEd/PropertyControls/PropertyControl.h>

namespace pulsared
{

    static void _Property2Name()
    {
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 120);
        ImGui::AlignTextToFramePadding();
    }
    static void _Property2Value()
    {
        ImGui::NextColumn();
    }
    static void _Property2End()
    {
        ImGui::Columns(1);
    }


    static constexpr int kTableRowHeight = 30;

    void PropertiesNodePanel::OnDrawImGui()
    {
        auto selectedObj = EditorSelection::Selection.GetSelected();

        Node_ref selected;
        if (cltypeof<Node>()->IsInstanceOfType(selectedObj.GetPtr()))
        {
            selected = selectedObj;
        }
        else
        {
            return;
        }

        static char name[255];
        strcpy_s(name, 255, selected->GetName().c_str());

        ImGui::Spacing();
        {
            bool is_active = selected->GetIsActiveSelf();
            ImGui::Checkbox("##active", &is_active);
            if (is_active != selected->GetIsActiveSelf())
            {
                selected->SetIsActiveSelf(is_active);
            }
            ImGui::SameLine();
            //ImGui::PushItemWidth(-1);
            if (ImGui::InputText("##Name", name, 255))
            {
                if (selected->GetName() != name)
                {
                    selected->SetName(name);
                }
            }

            ImGui::SameLine();
            ImGui::Checkbox("##debug", &m_debugMode);
            ImGui::SameLine();
            ImGui::Text("DebugMode");

            //ImGui::PopItemWidth();
        }

        ImGui::Spacing();

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Components");
        ImGui::SameLine();
        ImGui::Button("Add Component", {-FLT_MIN, 20});

        auto componentArr = selected->GetAllComponentArray();
        for (auto& comp : componentArr)
        {
            if(!IsValid(comp)) continue;

            bool opened = true;
            const bool dontDestroy = comp->HasObjectFlags(OF_DontDestroy);
            string componentFriendlyName = ComponentInfoManager::GetFriendlyComponentName(comp->GetType());

            ImGui::PushID(comp.GetPtr());
            if (ImGui::CollapsingHeader(
                componentFriendlyName.c_str(),
                dontDestroy ? nullptr : &opened,
                ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto componentGuid = comp->GetObjectHandle().to_string();
                Type* componentType = comp->GetType();
                auto fields = componentType->GetFieldInfos(TypeBinding::NonPublic);

                PImGui::ObjectFieldProperties(
                    comp->GetType(),
                    comp->GetType(),
                    comp.GetPtr(),
                    comp.GetPtr(), m_debugMode);
            }
            ImGui::PopID();

            if(!opened)
            {
                DestroyObject(comp);
            }
        }

        if (ImGui::CollapsingHeader("__Node Infomation", nullptr, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::BeginDisabled();

            _Property2Name();
            ImGui::Text("Type");
            _Property2Value();
            ImGui::Text(selected->GetType()->GetName().c_str());
            _Property2End();

            _Property2Name();
            ImGui::Text("Handle");
            _Property2Value();
            ImGui::Text(selected->GetObjectHandle().to_string().c_str());
            _Property2End();

            ImGui::EndDisabled();
        }

        ImGui::Spacing();
    }
} // namespace pulsared