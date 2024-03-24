#include "EditorWorld.h"

#include <Pulsar/Scene.h>
#include <Pulsar/World.h>
#include <PulsarEd/EditorNode.h>
#include <PulsarEd/Windows/OutlinerWindow.h>

namespace pulsared
{
    static void _Show(EditorWorld* world, List_sp<Node_ref> nodes)
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
            if (world->GetSelection().GetSelected() == node)
            {
                base_flags |= ImGuiTreeNodeFlags_Selected;
            }
            bool is_editor_node = false;
            if (node->HasObjectFlags(OF_NoPack))
            {
                is_editor_node = true;
                ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(0.2, 0.8, 0.8, 1));
            }
            string name = node->GetName();
            if (is_editor_node)
                name.append(" (EditorOnly)");

            ImGui::PushID(node->GetObjectHandle().to_string().c_str());
            bool isOpened = ImGui::TreeNodeEx(node.GetPtr(), base_flags, name.c_str());

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {
                world->GetSelection().Clear();
                world->GetSelection().Select(node);
            }

            if (isOpened)
            {
                auto childNodes = mksptr(new List<Node_ref>);
                childNodes->reserve(children->size());
                for (auto& child : *children)
                {
                    childNodes->push_back(child->GetAttachedNode());
                }
                _Show(world, childNodes);
                ImGui::TreePop();
            }
            ImGui::PopID();
            if (is_editor_node)
            {
                ImGui::PopStyleColor();
            }
        }
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
            if (opened)
            {
                _Show(world, currentScene->GetRootNodes());
                ImGui::TreePop();
            }
        }

    }
}