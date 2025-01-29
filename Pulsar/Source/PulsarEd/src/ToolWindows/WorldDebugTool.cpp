#include "AssetDatabase.h"
#include "Pulsar/Scene.h"

#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/ToolWindows/WorldDebugTool.h>

namespace pulsared
{

    WorldDebugTool::WorldDebugTool()
    {
        m_winSize = {800, 500};
    }
    static void RenderNodes(Node_ref node)
    {
        if (ImGui::TreeNode(node->GetName().c_str()))
        {
            for (auto element : *node->GetTransform()->GetChildren())
            {
                RenderNodes(element->GetNode());
            }
            ImGui::TreePop();
        }
    }
    void WorldDebugTool::OnDrawImGui(float dt)
    {
        for (auto world : World::GetAllWorlds())
        {
            if (ImGui::TreeNode(world->GetWorldName().c_str()))
            {
                for (int i = 0; i < world->GetSceneCount(); ++i)
                {
                    if (ImGui::TreeNode(world->GetScene(i)->GetName().c_str()))
                    {
                        for (auto rootNode : *world->GetScene(i)->GetRootNodes())
                        {
                            RenderNodes(rootNode);
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
        }
    }
} // namespace pulsared