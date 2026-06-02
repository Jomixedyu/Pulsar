#include "EditorWorld.h"
#include "Editors/EditorWindow.h"
#include "Editors/SceneEditor/SceneEditor.h"

#include <Pulsar/Scene.h>
#include <Pulsar/World.h>
#include <Pulsar/Prefab.h>
#include <Pulsar/Components/CameraComponent.h>
#include <PulsarEd/EditorNode.h>
#include <PulsarEd/Editors/CommonPanel/OutlinerWindow.h>
#include <PulsarEd/DragInfo.h>
#include <PulsarEd/AssetDatabase.h>

namespace pulsared
{
    static const char* _GetNodeIcon(pulsar::Node* node)
    {
        for (auto& comp : node->GetAllComponentArray())
        {
            if (auto attr = comp->GetType()->GetAttribute<pulsar::ComponentIconAttribute>())
            {
                return attr->GetIcon();
            }
        }
        return ICON_FK_CIRCLE_O;
    }

    static bool _IsDescendantOf(pulsar::Node* ancestor, pulsar::Node* descendant)
    {
        if (ancestor == descendant)
            return true;
        for (auto& child : *ancestor->GetTransform()->GetChildren())
        {
            if (_IsDescendantOf(child->GetNode().GetPtr(), descendant))
                return true;
        }
        return false;
    }

    static void _SetNodeParent(pulsar::Node* node, pulsar::Node* parent)
    {
        auto transform = node->GetTransform();
        auto worldPos = transform->GetWorldPosition();
        auto worldRot = transform->GetWorldRotation();
        auto worldScl = transform->GetWorldScale();

        if (parent)
        {
            node->SetParent(pulsar::ObjectPtr<pulsar::Node>::UnsafeCreate(parent->GetObjectHandle()));
        }
        else
        {
            if (transform->GetParent())
            {
                auto collection = node->GetOwnerNodeCollection();
                transform->SetParent(nullptr);
                if (collection)
                    collection->RegisterRootNode(pulsar::ObjectPtr<pulsar::Node>::UnsafeCreate(node->GetObjectHandle()));
            }
        }

        transform->SetWorldPosition(worldPos);
        transform->SetWorldRotation(worldRot);
        transform->SetWorldScale(worldScl);
    }

    // Forward declare
    static void _HandlePrefabDrop(pulsar::NodeCollection* scene);

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
            bool is_inactive = !node->GetIsActive();

            if (node->HasObjectFlags(OF_NoPack))
            {
                is_editor_node = true;
                ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.8f, 1.f));
            }
            else if (is_prefab_node)
            {
                ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.f));
            }
            else if (is_inactive)
            {
                ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.f));
            }
            string name = string(_GetNodeIcon(node.GetPtr())) + " " + node->GetName();
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

            if (ImGui::BeginPopupContextItem("OutlinerNodeContext"))
            {
                if (is_prefab_node)
                {
                    if (ImGui::MenuItem("Unpack Prefab"))
                    {
                        auto collection = node->GetOwnerNodeCollection();
                        if (collection)
                        {
                            collection->UnpackTemplateInstanceByNode(node);
                        }
                    }
                }
                ImGui::EndPopup();
            }

            // Drag source
            if (ImGui::BeginDragDropSource())
            {
                pulsar::Node* ptr = node.GetPtr();
                ImGui::SetDragDropPayload("OUTLINER_NODE", &ptr, sizeof(pulsar::Node*));
                ImGui::Text("Move %s", node->GetName().c_str());
                ImGui::EndDragDropSource();
            }

            // Drop target: parent the dragged node to this node
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OUTLINER_NODE"))
                {
                    pulsar::Node* dragged = *(pulsar::Node**)payload->Data;
                    if (dragged && dragged != node.GetPtr()
                        && dragged->GetOwnerNodeCollection() == node->GetOwnerNodeCollection()
                        && !_IsDescendantOf(dragged, node.GetPtr()))
                    {
                        _SetNodeParent(dragged, node.GetPtr());
                    }
                }
                _HandlePrefabDrop(node->GetOwnerNodeCollection());
                ImGui::EndDragDropTarget();
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
            if (is_editor_node || is_prefab_node || is_inactive)
            {
                ImGui::PopStyleColor();
            }
        }
    }

    // 处理 WorkspaceWindow 拖来的 Prefab，放入指定 scene
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
        auto sceneEditor = dynamic_cast<SceneEditor*>(GetParentEditorWindow()->GetEditor());
        auto world = sceneEditor ? dynamic_cast<EditorWorld*>(sceneEditor->GetPreviewWorld()) : nullptr;
        if (!world)
        {
            return;
        }

        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed;

        for (int i = 0; i < world->GetSceneCount(); i++)
        {
            auto currentScene = world->GetScene(i);
            bool isFocus = (currentScene == world->GetFocusScene());

            string label = string(ICON_FK_MAP) + " " + currentScene->GetName();
            if (auto asset = cast<AssetObject>(currentScene))
            {
                if (AssetDatabase::IsDirty(asset))
                {
                    label += " *";
                }
            }
            if (isFocus)
            {
                label += "  (Focus Scene)";
            }
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{0.08f, 0.08f, 0.10f, 1.0f});
            bool opened = ImGui::TreeNodeEx(label.c_str(), base_flags);
            ImGui::PopStyleColor();

            // drop target：挂在 scene TreeNode header 上（节点拖来设为 root）
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OUTLINER_NODE"))
                {
                    pulsar::Node* dragged = *(pulsar::Node**)payload->Data;
                    if (dragged && dragged->GetOwnerNodeCollection() == currentScene.GetPtr())
                        _SetNodeParent(dragged, nullptr);
                }
                _HandlePrefabDrop(currentScene.GetPtr());
                ImGui::EndDragDropTarget();
            }

            if (opened)
            {
                auto rootNodesCopy = mksptr(new List<ObjectPtr<Node>>(*currentScene->GetRootNodes()));
                _Show(world, rootNodesCopy);
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
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OUTLINER_NODE"))
                    {
                        pulsar::Node* dragged = *(pulsar::Node**)payload->Data;
                        if (dragged && dragged->GetOwnerNodeCollection() == focusScene.GetPtr())
                            _SetNodeParent(dragged, nullptr);
                    }
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
