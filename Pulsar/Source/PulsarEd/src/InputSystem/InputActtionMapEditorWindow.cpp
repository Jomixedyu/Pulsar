#include "InputSystem/InputActtionMapEditorWindow.h"

#include "ImGuiExt.h"
#include "PropertyControls/PropertyControl.h"

namespace pulsared
{

    void InputActionMapEditorWindow::OnDrawAssetEditor(float dt)
    {
        base::OnDrawAssetEditor(dt);

        ImGui::Columns(2);

        auto inputMap = cref_cast<InputActionMap>(m_assetObject).GetPtr();

        bool isDirty = false;

        bool openedActions = ImGui::BeginChild("InputActions", {}, {}, ImGuiWindowFlags_MenuBar);
        if (openedActions)
        {
            if (ImGui::BeginMenuBar())
            {
                ImGui::Text("Actions");
                if (ImGui::Button("+"))
                {
                    auto newName = StringUtil::GetUniqueName("NewAction", *inputMap->GetActionNames());
                    inputMap->NewAction(newName);
                    isDirty = true;
                }
                if (ImGui::Button("-"))
                {
                    inputMap->RemoveActionAt(m_selectedActionIndex);
                    isDirty = true;
                }
                ImGui::EndMenuBar();
            }
            if (inputMap)
            {
                ImGui::BeginListBox("Action List", { -FLT_MIN, -FLT_MIN });
                auto count = inputMap->GetActionCount();
                for (int i = 0; i < count; ++i)
                {
                    ImGui::PushID(i);
                    bool selected = ImGui::Selectable(inputMap->GetActionNameAt(i).c_str(), m_selectedActionIndex == i);
                    if (selected)
                    {
                        m_selectedActionIndex = i;
                    }
                    ImGui::PopID();
                }
                ImGui::EndListBox();
            }
        }


        ImGui::EndChild();

        ImGui::NextColumn();

        bool openedProperty = ImGui::BeginChild("InputActionProperty", {}, {});
        if (openedProperty)
        {
            if (inputMap->IsValidIndex(m_selectedActionIndex))
            {
                auto actionName = inputMap->GetActionNameAt(m_selectedActionIndex);
                auto action = inputMap->GetActionAt(m_selectedActionIndex);

                if (PImGui::BeginPropertyLines())
                {
                    auto inputAlias = mkbox(actionName);
                    if (PImGui::PropertyLine("Alias", cltypeof<String>(), inputAlias.get()))
                    {
                        inputMap->SetActionNameAt(m_selectedActionIndex, UnboxUtil::Unbox<string>(inputAlias));
                        isDirty = true;
                    }

                    auto inputType = mkbox(action->m_valueType);
                    if (PImGui::PropertyLine("InputType", cltypeof<BoxingInputValueType>(), inputType.get()))
                    {
                        action->m_valueType = UnboxUtil::Unbox<InputValueType>(inputType);
                        isDirty = true;
                    }

                    PImGui::EndPropertyLines();
                }

                ImGui::Text("Bindings");
                ImGui::SameLine();
                if (ImGui::Button("+"))
                {
                    ImGui::OpenPopup("InputActionMapEditorWindow.AddBinding");
                }

                if (ImGui::BeginPopup("InputActionMapEditorWindow.AddBinding"))
                {
                    if (ImGui::Button("Keyboard"))
                    {
                        action->m_bindings.push_back(mksptr(new InputActionKeyboardBinding));
                        isDirty = true;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                array_list<SPtr<InputActionBinding>> pendingRemoveList;
                auto bindingSize = action->m_bindings.size();
                for (int i = 0; i < bindingSize; ++i)
                {
                    ImGui::PushID(i);
                    auto& item = action->m_bindings.at(i);
                    bool existsItem = true;
                    //bindings
                    if (ImGui::CollapsingHeader(item->ToString().c_str(), &existsItem, ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        auto bindingType = item->GetType();
                        isDirty |= PImGui::ObjectFieldProperties(bindingType, bindingType, item.get(), inputMap);
                    }
                    if (!existsItem)
                    {
                        pendingRemoveList.push_back(item);
                        isDirty = true;
                    }
                    ImGui::PopID();
                }

                for (auto& item : pendingRemoveList)
                {
                    std::erase(action->m_bindings, item);
                }

            }

        }

        ImGui::EndChild();

        if (isDirty)
        {
            AssetDatabase::MarkDirty(inputMap);
        }

        ImGui::Columns(1);


    }
    void InputActionMapEditorWindow::OnOpen()
    {
        base::OnOpen();
    }
    void InputActionMapEditorWindow::OnClose()
    {
        base::OnClose();
    }

} // namespace pulsared