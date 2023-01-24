#include <ApatiteEd/Windows/OutlinerWindow.h>
#include <Apatite/World.h>
#include <Apatite/Scene.h>
#include <ApatiteEd/EditorNode.h>
#include <ApatiteEd/EditorSelection.h>

namespace apatiteed
{
    static void _Show(List_sp<Node_sp> nodes)
    {
        for (auto& node : *nodes)
        {
            List_sp<Node_sp> children = mksptr(new List<Node_sp>);
            node->GetChildren(children);
            ImGuiTreeNodeFlags base_flags =
                ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_OpenOnDoubleClick |
                ImGuiTreeNodeFlags_SpanFullWidth;


            if (children->size() == 0)
            {
                base_flags |= ImGuiTreeNodeFlags_Leaf;
            }
            if (EditorSelection::Selection.GetSelected() == node)
            {
                base_flags |= ImGuiTreeNodeFlags_Selected;
            }
            bool is_editor_node = false;
            if (cltypeof<EditorNode>()->IsInstanceOfType(node.get()))
            {
                is_editor_node = true;
                ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(0.2, 0.6, 0.5, 1));
            }
            string name = node->get_name();
            if (is_editor_node)
                name.append(" (EditorOnly)");
            ImGui::PushID(node->get_object_id().to_string().c_str());
            if (ImGui::TreeNodeEx(name.c_str(), base_flags))
            {
                if (ImGui::IsItemClicked())
                {
                    EditorSelection::Selection.Clear();
                    EditorSelection::Selection.Select(node);
                }
                _Show(children);
                ImGui::TreePop();
            }
            ImGui::PopID();
            if (is_editor_node)
            {
                ImGui::PopStyleColor();
            }
        }
    }
    void OutlinerWindow::OnDrawImGui()
    {
        auto world = World::Current();
        if (!world)
        {
            return;
        }

        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
        if (ImGui::TreeNodeEx("SceneRoot", base_flags))
        {
            _Show(World::Current()->scene->get_root_nodes());
            ImGui::TreePop();
        }
    }
}