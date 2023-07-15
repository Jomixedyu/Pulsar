#include <PulsarEd/Windows/WorkspaceWindow.h>
#include <PulsarEd/AssetDatabase.h>
#include <PulsarEd/Workspace.h>

namespace pulsared
{
    static void ShowContent(FileNode* node)
    {
        ImGuiTreeNodeFlags base_flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_SpanFullWidth;
        if (!node->is_folder)
        {
            base_flags |= ImGuiTreeNodeFlags_Leaf;
        }
        if (ImGui::TreeNodeEx(node->name.c_str(), base_flags))
        {
            if (node->is_folder)
            {
                for (auto& i : node->children)
                {
                    ShowContent(&i);
                }
                
            }
            ImGui::TreePop();
        }
    }
    void WorkspaceWindow::OnDrawImGui()
    {

        ImGui::InputText("Search", this->search_buf, sizeof(this->search_buf));


        if (!Workspace::IsOpened())
        {
            return;
        }

        ShowContent(&AssetDatabase::FileTree);
    }
    WorkspaceWindow::WorkspaceWindow()
    {
        ::memset(this->search_buf, 0, sizeof(this->search_buf));
    }
}