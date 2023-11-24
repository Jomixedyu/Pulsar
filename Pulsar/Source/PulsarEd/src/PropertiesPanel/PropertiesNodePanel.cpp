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

    static bool _PropertyLine(const string& name, Type* type, Object* obj)
    {
        _Property2Name();
        ImGui::Text(StringUtil::FriendlyName(name).c_str());
        _Property2Value();
        bool changed = PropertyControlManager::ShowProperty(name, type, obj);
        _Property2End();
        return changed;
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

        bool is_active = selected->GetIsActiveSelf();
        ImGui::Checkbox("##active", &is_active);
        if (is_active != selected->GetIsActiveSelf())
        {
            selected->SetIsActiveSelf(is_active);
        }
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        if (ImGui::InputText("##Name", name, 255))
        {
            if (selected->GetName() != name)
            {
                selected->SetName(name);
            }
        }
        ImGui::PopItemWidth();

        ImGui::Spacing();

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Components");
        ImGui::SameLine();
        ImGui::Button("Add Component", { -FLT_MIN,20 });

        for (auto& comp : selected->GetAllComponentArray())
        {
            static bool opened = true;

            string componentFriendlyName = _GetComponentDisplayName(comp.GetPtr());

            ImGui::PushID(comp.GetPtr());
            if (ImGui::CollapsingHeader(componentFriendlyName.c_str(), &opened, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto componentGuid = comp->GetObjectHandle().to_string();
                Type* componentType = comp->GetType();
                auto fields = componentType->GetFieldInfos(TypeBinding::NonPublic);

                for (auto& fieldInfo : fields)
                {
                    if (fieldInfo->IsDefinedAttribute(cltypeof<HideInComponentPropertyAttribute>()))
                    {
                        continue;
                    }

                    auto fieldInstShared = fieldInfo->GetValue(comp.GetPtr());

                    const bool isObjectPtr = fieldInstShared->GetType() == BoxingObjectPtrBase::StaticType();

                    if (isObjectPtr)
                    {
                        auto handle = UnboxUtil::Unbox<ObjectPtrBase>(fieldInstShared).GetHandle();
                        auto fieldInst = RuntimeObjectWrapper::GetObject(handle);
                        _PropertyLine(fieldInfo->GetName(), fieldInfo->GetWrapType(), fieldInst);
                    }
                    else
                    {
                        if (_PropertyLine(fieldInfo->GetName(), fieldInstShared->GetType(), fieldInstShared.get()))
                        {
                            fieldInfo->SetValue(comp.GetPtr(), fieldInstShared);
                            comp->PostEditChange(fieldInfo);
                        }

                    }

                }

                ImGui::BeginDisabled();

                _Property2Name();
                ImGui::Text("Type");
                _Property2Value();
                ImGui::Text(comp->GetType()->GetName().c_str());
                _Property2End();

                _Property2Name();
                ImGui::Text("Handle");
                _Property2Value();
                ImGui::Text(comp->GetObjectHandle().to_string().c_str());
                _Property2End();

                ImGui::EndDisabled();
            }
            ImGui::PopID();

        }

        static bool nodeOpened = true;
        if (ImGui::CollapsingHeader("__Node Infomation", &nodeOpened, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
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
}