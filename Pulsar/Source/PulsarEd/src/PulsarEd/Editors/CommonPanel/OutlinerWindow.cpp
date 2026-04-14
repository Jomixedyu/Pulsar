#include "EditorWorld.h"

#include <Pulsar/Scene.h>
#include <Pulsar/World.h>
#include <Pulsar/Prefab.h>
#include <PulsarEd/EditorNode.h>
#include <PulsarEd/Editors/CommonPanel/OutlinerWindow.h>
#include <PulsarEd/DragInfo.h>
#include <PulsarEd/AssetDatabase.h>

namespace pulsared
{
    static void _Show(EditorWorld* world, List_sp<ObjectPtr<Node>> nodes)
    {
        for (auto& node : *nodes)
        {
            auto children = node->GetTransform()->GetChildren();

            ImGuiTreeNodeFlags base_flags =
                ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_OpenOnDoubleClick |
                ImGuiTreeNodeFlags_SpanFullWidth;


            if (children->size() == 0)
            {
                base_flags |= ImGuiTreeNodeFlags_Leaf;
            }
            if (world->GetSelection().IsSelected(node))
            {
                base_flags |= ImGuiTreeNodeFlags_Selected;
            }
            bool is_editor_node = false;
            bool is_prefab_node = node->IsTemplateInstance();

            if (node->HasObjectFlags(OF_NoPack))
            {
                is_editor_node = true;
                ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.8f, 1.f));
            }
            else if (is_prefab_node)
            {
                ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.f));
            }
            string name = node->GetName();
            if (is_editor_node)
                name.append(" (EditorOnly)");

            ImGui::PushID(node->GetObjectHandle().to_string().c_str());
            bool isOpened = ImGui::TreeNodeEx(node.GetPtr(), base_flags, name.c_str());

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {
                if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
                {
                    world->GetSelection().Select(node);
                }
                else if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
                {
                    world->GetSelection().UnSelect(node);
                }
                else
                {
                    world->GetSelection().Clear();
                    world->GetSelection().Select(node);
                }

            }

            if (isOpened)
            {
                auto childNodes = mksptr(new List<ObjectPtr<Node>>);
                childNodes->reserve(children->size());
                for (auto& child : *children)
                {
                    childNodes->push_back(child->GetNode());
                }
                _Show(world, childNodes);
                ImGui::TreePop();
            }
            ImGui::PopID();
            if (is_editor_node || is_prefab_node)
            {
                ImGui::PopStyleColor();
            }
        }
    }
    // 处理从 WorkspaceWindow 拖来的 Prefab，放入指定 scene
    static void _HandlePrefabDrop(pulsar::NodeCollection* scene)
    {
        const ImGuiPayload* peekPayload = ImGui::GetDragDropPayload();
        if (!peekPayload || peekPayload->DataType != AssetObjectDragInfo::Name)
            return;

        const auto* dragData = static_cast<const AssetObjectDragInfo*>(peekPayload->Data);
        if (dragData->Type != cltypeof<pulsar::Prefab>())
            return;

        const ImGuiPayload* accepted = ImGui::AcceptDragDropPayload(AssetObjectDragInfo::Name.data());
        if (!accepted)
            return;

        const auto* acceptedData = static_cast<const AssetObjectDragInfo*>(accepted->Data);
        auto assetPath = AssetDatabase::GetPathByGuid(acceptedData->AssetGuid);
        auto assetObj  = AssetDatabase::LoadAssetAtPath(assetPath);
        auto prefab    = cast<pulsar::Prefab>(assetObj);
        if (!prefab)
            return;

        // 直接通过 NodeCollection 实例化 Prefab
        scene->AddTemplateInstance(prefab);
    }

    void OutlinerWindow::OnDrawImGui(float dt)
    {
        auto world = dynamic_cast<EditorWorld*>(EditorWorld::GetPreviewWorld());
        if (!world)
        {
            return;
        }

        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

        for (int i = 0; i < world->GetSceneCount(); i++)
        {
            auto currentScene = world->GetScene(i);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{0,0,0,1});
            bool opened = ImGui::TreeNodeEx(currentScene->GetName().c_str(), base_flags);
            ImGui::PopStyleColor();

            // drop target：挂在 scene TreeNode header 上
            if (ImGui::BeginDragDropTarget())
            {
                _HandlePrefabDrop(currentScene.GetPtr());
                ImGui::EndDragDropTarget();
            }

            if (opened)
            {
                _Show(world, currentScene->GetRootNodes());
                ImGui::TreePop();
            }
        }

        // drop target：窗口空白处也能接收（用不可见的全窗口矩形）
        {
            auto focusScene = world->GetFocusScene();
            if (focusScene)
            {
                ImVec2 winPos  = ImGui::GetWindowPos();
                ImVec2 winSize = ImGui::GetWindowSize();
                ImGui::SetNextItemAllowOverlap();
                ImGui::SetCursorScreenPos(winPos);
                ImGui::InvisibleButton("##outliner_drop_zone", winSize, ImGuiButtonFlags_None);
                if (ImGui::BeginDragDropTarget())
                {
                    _HandlePrefabDrop(focusScene.GetPtr());
                    ImGui::EndDragDropTarget();
                }
            }
        }
        if (ImGui::IsWindowFocused())
        {
            if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_Delete, false))
            {
                auto selection = world->GetSelection().GetSelection();

                for (auto& item : selection)
                {
                    if (!item) continue;
                    auto collection = item->GetOwnerNodeCollection();
                    if (item->IsTemplateInstance())
                    {
                        // 删除整个 template 实例
                        collection->RemoveTemplateInstanceByNode(item);
                    }
                    else
                    {
                        collection->RemoveNode(item);
                    }
                }
            }
        }

    }
}