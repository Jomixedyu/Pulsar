#include "Windows/ShelfBarWindow.h"

#include "Menus/ISubMenu.h"
#include "Menus/Menu.h"
#include "Menus/MenuEntrySubMenu.h"

namespace pulsared
{
    static void RenderToolbar(ISubMenu* menu)
    {
        if (!menu)
            return;

        if (ImGui::BeginTabBar(menu->GetMenuName().c_str()))
        {
            for (auto& entry : menu->GetEntries())
            {
                if (auto submenu = interface_cast<ISubMenu>(entry.get()))
                {
                    if (ImGui::BeginTabItem(submenu->GetMenuName().c_str()))
                    {
                        int itemIndex = 0;
                        for (auto& submenuEntry : submenu->GetEntries())
                        {
                            if (auto btn = sptr_cast<MenuEntryButton>(submenuEntry))
                            {
                                ImGui::PushID(itemIndex);
                                if (ImGui::Button(btn->DisplayName.c_str(), {0, -FLT_MIN}))
                                {
                                }
                                ImGui::PopID();
                                ImGui::SameLine();
                            }
                            ++itemIndex;
                        }
                        ImGui::EndTabItem();
                    }
                }
            }
            ImGui::EndTabBar();
        }
    }

    ImGuiWindowFlags ShelfBarWindow::GetGuiWindowFlags() const
    {
        return ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
    }
    void ShelfBarWindow::OnDrawImGui(float dt)
    {
        base::OnDrawImGui(dt);
        RenderToolbar(MenuManager::GetOrAddMenu("ToolBar"));
    }
    ShelfBarWindow::ShelfBarWindow()
    {
        auto lighting = mksptr(new MenuEntrySubMenu("Lighting"));
        MenuManager::GetOrAddMenu("ToolBar")->AddEntry(lighting);

        auto rendering = mksptr(new MenuEntrySubMenu("Rendering"));
        rendering->AddEntry(mksptr(new MenuEntryButton("Play")));
        rendering->AddEntry(mksptr(new MenuEntryButton("Stop")));
        MenuManager::GetOrAddMenu("ToolBar")->AddEntry(rendering);
    }
} // namespace pulsared