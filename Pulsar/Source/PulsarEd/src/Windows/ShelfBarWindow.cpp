#include "Windows/ShelfBarWindow.h"

#include "Menus/ISubMenu.h"
#include "Menus/Menu.h"
#include "Menus/MenuEntrySubMenu.h"
#include "imgui/imgui_internal.h"

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
                            ImGui::PushID(itemIndex);
                            if (auto btn = sptr_cast<MenuEntryButton>(submenuEntry))
                            {
                                if (ImGui::Button(btn->DisplayName.c_str(), {0, -FLT_MIN}))
                                {
                                }
                            }
                            else if(sptr_cast<MenuEntrySeparate>(submenuEntry))
                            {
                                ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
                            }
                            ImGui::PopID();
                            ImGui::SameLine();
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
        auto rendering = mksptr(new MenuEntrySubMenu("App"));
        rendering->AddEntry(mksptr(new MenuEntryButton("Play")));
        rendering->AddEntry(mksptr(new MenuEntryButton("Stop")));
        rendering->AddEntry(mksptr(new MenuEntrySeparate("0")));
        rendering->AddEntry(mksptr(new MenuEntryButton("Build")));
        MenuManager::GetOrAddMenu("ToolBar")->AddEntry(rendering);

        auto lighting = mksptr(new MenuEntrySubMenu("Rendering"));
        MenuManager::GetOrAddMenu("ToolBar")->AddEntry(lighting);
    }
} // namespace pulsared