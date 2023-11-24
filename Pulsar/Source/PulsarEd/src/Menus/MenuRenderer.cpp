#include <PulsarEd/Menus/MenuRenderer.h>
#include <PulsarEd/EditorUI.h>

namespace pulsared
{
    static void RenderSeparator(const string& str)
    {
        ImGui::SeparatorText(str.c_str());
    }

    void MenuRenderer::RenderMenu(ISubMenu* menu, MenuContexts_sp ctxs)
    {
        bool lastSep = false;
        string lastSepStr;

        for (auto& entry : menu->GetEntries())
        {
            if (auto separate = sptr_cast<MenuEntrySeparate>(entry))
            {
                lastSep = true;
                lastSepStr = entry->DisplayName;
            }
            else if (auto subMenu = sptr_cast<MenuEntrySubMenu>(entry))
            {
                if (lastSep)
                {
                    RenderSeparator(lastSepStr);
                    lastSep = false;
                }

                if (ImGui::BeginMenu(subMenu->DisplayName.c_str()))
                {
                    RenderMenu(subMenu.get(), ctxs);
                    ImGui::EndMenu();
                }
            }
            else if (auto btn = sptr_cast<MenuEntryButton>(entry))
            {
                ctxs->EntryName = btn->Name;
                if (lastSep)
                {
                    RenderSeparator(lastSepStr);
                    lastSep = false;
                }

                bool visibility = true;
                if (btn->Visibility)
                {
                    visibility = btn->Visibility->Invoke(ctxs);
                }
                if (!visibility)
                {
                    continue;
                }

                bool operate = true;
                if (btn->CanOperate)
                {
                    operate = btn->CanOperate->Invoke(ctxs);
                }

                if (!operate) ImGui::BeginDisabled();

                if (ImGui::MenuItem(entry->DisplayName.c_str()))
                {
                    if (btn->Action)
                        btn->Action->Invoke(ctxs);
                }

                if (!operate) ImGui::EndDisabled();

            }
        }
    }
}