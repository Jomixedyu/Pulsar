#include <ApatiteEd/Windows/OutlinerWindow.h>
#include <Apatite/World.h>

namespace apatiteed
{
    void OutlinerWindow::OnDrawImGui()
    {
        //static bool open;
        //ImGui::ShowDemoWindow(&open);

        auto world = World::Current();
        if (!world)
        {
            return;
        }

        
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