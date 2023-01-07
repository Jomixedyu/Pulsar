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
            ImGuiTreeNodeFlags_Selected;

            if (children->size() == 0)
            {
                base_flags |= ImGuiTreeNodeFlags_Leaf;
            }
            if (EditorSelection::Selection.GetSelected() == node)
            {
                base_flags |= ImGuiTreeNodeFlags_Selected;
            }
            if (ImGui::TreeNodeEx(node->get_name().c_str(), base_flags))
            {
                if (ImGui::IsItemClicked())
                {
                    EditorSelection::Selection.Select(node);
                }
                _Show(children);
                ImGui::TreePop();
            }
        }
    }
    void OutlinerWindow::OnDrawImGui()
    {
        static bool open;
        ImGui::ShowDemoWindow(&open);

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