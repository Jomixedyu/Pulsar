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
        if (!menu) return;
        if (!ctxs)
        {
            ctxs = mksptr(new MenuContexts);
        }
        bool lastSep = false;
        string lastSepStr;

        auto entries = menu->GetEntries();
        std::ranges::sort(entries, [](const MenuEntry_sp& a, const MenuEntry_sp& b){ return a->Priority < b->Priority; });

        auto& style = ImGui::GetStyle();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {style.ItemSpacing.x, 8});

        for (auto& entry : entries)
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
            else if (auto checkEntry = sptr_cast<MenuEntryCheck>(entry))
            {
                ctxs->EntryName = checkEntry->Name;

                bool checked = checkEntry->IsChecked;
                if (checkEntry->GetCheckedAction)
                {
                    checked = checkEntry->GetCheckedAction->Invoke(ctxs);
                }
                bool originalChecked = checked;

                ImGui::MenuItem(checkEntry->DisplayName.c_str(), 0,  &checked);

                if(checked != originalChecked)
                {
                    if(checkEntry->CheckedAction && checkEntry->CheckedAction->IsValid())
                    {
                        checkEntry->IsChecked = checked;
                        checkEntry->CheckedAction->Invoke(ctxs, checked);
                    }
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
        ImGui::PopStyleVar();
    }
}