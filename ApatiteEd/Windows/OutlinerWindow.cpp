#include <apatiteed/Windows/OutlinerWindow.h>

namespace apatiteed
{
    void OutlinerWindow::OnDrawImGui()
    {
        static bool open;
        ImGui::ShowDemoWindow(&open);

        if (ImGui::TreeNode("SceneRoot"))
        {
            if (ImGui::TreeNode("GameObject"))
            {

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }
}