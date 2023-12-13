#include <PulsarEd/Tools/MenuDebugTool.h>
#include <PulsarEd/Menus/Menu.h>
namespace pulsared
{
    static void _ShowMenu(ISubMenu* menu)
    {
        if (ImGui::TreeNodeEx(menu, 0, menu->GetMenuName().c_str()))
        {
            for (auto& entry : menu->GetEntries())
            {
                if (auto subMenu = interface_cast<ISubMenu>(entry.get()))
                {
                    _ShowMenu(subMenu);
                }
                else
                {
                    auto str = StringUtil::Concat(entry->Name, " (", entry->GetType()->GetShortName(), " : ", entry->DisplayName, ")");

                    if (ImGui::TreeNodeEx(entry.get(), ImGuiTreeNodeFlags_Leaf, str.c_str()))
                    {
                        ImGui::TreePop();
                    }
                }
            }
            ImGui::TreePop();
        }
    }
    void MenuDebugTool::OnDrawImGui()
    {
        for (auto& menu : MenuManager::GetMenus())
        {
            _ShowMenu(menu);
        }
    }
}