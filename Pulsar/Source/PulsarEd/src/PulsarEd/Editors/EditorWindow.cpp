#include "Editors/EditorWindow.h"

#include "Editors/Editor.h"
#include "Editors/EditorRegistry.h"
#include "Menus/ISubMenu.h"
#include "Menus/Menu.h"
#include "Menus/MenuEntrySubMenu.h"
#include "Menus/MenuRenderer.h"
#include "Windows/EditorWindowManager.h"
#include "Windows/PanelWindow.h"

namespace pulsared
{
    ImGuiWindowFlags EditorWindow::GetGuiWindowFlags() const
    {
        return ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;
    }
    ImGuiWindowClass EditorWindow::GetGuiWindowClass() const
    {
        ImGuiWindowClass winClass{};
        winClass.DockingAllowUnclassed = false;
        winClass.ClassId = ImGui::GetID("EditorWindow");

        return winClass;
    }
    void EditorWindow::OnOpen()
    {
        base::OnOpen();
    }
    void EditorWindow::OnClose()
    {
        for (auto& panel : m_openedPanels)
        {
            panel->OnClose();
            panel->SetOpened(false);
        }
        m_openedPanels.clear();
        base::OnClose();
    }

    void EditorWindow::OnDrawImGui(float dt)
    {
        base::OnDrawImGui(dt);

        // render menu
        if (auto editor = GetEditor())
        {
            auto ctxs = mksptr(new MenuContexts);

            auto ctx = mksptr(new EditorWindowMenuContext);
            ctx->m_editorWindow = this;

            ctxs->Contexts.push_back(ctx);

            OnBuildMenuContexts(ctxs);

            if (ImGui::BeginMenuBar())
            {
                auto mergedMenu = mksptr(new MenuEntrySubMenu("MergedMenuBar"));
                hash_map<string, MenuEntrySubMenu_sp> mergedSubMenus;
                hash_set<string> mergedEntryNames;

                auto type = editor->GetType();
                while (type && type != cltypeof<Object>())
                {
                    if (auto* parentEditor = EditorRegistry::GetEditor(type))
                    {
                        auto menu = MenuManager::GetMenu(parentEditor->GetMenuName());
                        if (menu)
                        {
                            for (auto& entry : menu->GetEntries())
                            {
                                if (auto subMenu = sptr_cast<MenuEntrySubMenu>(entry))
                                {
                                    auto it = mergedSubMenus.find(subMenu->Name);
                                    if (it == mergedSubMenus.end())
                                    {
                                        auto newSub = mksptr(new MenuEntrySubMenu(subMenu->Name, subMenu->DisplayName));
                                        newSub->Priority = subMenu->Priority;
                                        mergedSubMenus[subMenu->Name] = newSub;
                                        mergedMenu->AddEntry(newSub);
                                        it = mergedSubMenus.find(subMenu->Name);
                                    }
                                    for (auto& subEntry : subMenu->GetEntries())
                                    {
                                        bool found = false;
                                        for (auto& e : it->second->GetEntries())
                                        {
                                            if (e->Name == subEntry->Name) { found = true; break; }
                                        }
                                        if (!found) it->second->AddEntry(subEntry);
                                    }
                                }
                                else if (mergedEntryNames.insert(entry->Name).second)
                                {
                                    mergedMenu->AddEntry(entry);
                                }
                            }
                        }
                    }
                    type = type->GetBase();
                }

                MenuRenderer::RenderMenu(mergedMenu.get(), ctxs);
                ImGui::EndMenuBar();
            }
        }

        if (m_useDockspace)
        {
            auto dockId = ImGui::GetID("EditorDockspace");

            ImGuiWindowClass dockspaceClass{};
            dockspaceClass.ClassId = m_dockspaceClassId;
            dockspaceClass.DockingAllowUnclassed = false;

            ImGui::DockSpace(dockId, {}, {}, &dockspaceClass);
        }

    }

    void EditorWindow::DrawImGui(float dt)
    {
        auto name = StringUtil::Concat(to_string(GetWindowId()), "+panel");
        m_dockspaceClassId = ImGui::GetID(name.c_str());
        base::DrawImGui(dt);

        for (size_t i = 0; i < m_openedPanels.size(); )
        {
            auto& panel = m_openedPanels[i];
            if (!panel->GetIsOpened())
            {
                ++i;
                continue;
            }

            bool isOpened = true;
            const auto winName = StringUtil::Concat(panel->GetWindowDisplayName(), "###", panel->GetWindowName());
            auto winClass = panel->GetGuiWindowClass();
            ImGui::SetNextWindowClass(&winClass);
            auto winSize = panel->GetWinSize();
            ImGui::SetNextWindowSize(ImVec2{winSize.x, winSize.y});
            const bool isDrawable = ImGui::Begin(winName.c_str(), &isOpened, panel->GetGuiWindowFlags() | ImGuiWindowFlags_NoCollapse);
            if (isOpened)
            {
                panel->SetOpened(true);
                if (isDrawable)
                {
                    panel->OnDrawImGui(dt);
                }
                if (panel->GetAllowResize())
                {
                    const auto size = ImGui::GetWindowSize();
                    panel->SetWinSize(Vector2f{size.x, size.y});
                }
            }
            else
            {
                panel->OnClose();
                panel->SetOpened(false);
                m_openedPanels.erase(m_openedPanels.begin() + i);
                ImGui::End();
                continue;
            }
            ImGui::End();
            ++i;
        }
    }

    void EditorWindow::RegisterPanelType(Type* type)
    {
        if (type == nullptr)
            return;
        if (std::ranges::contains(m_registeredPanelTypes, type))
            return;
        if (!type->IsSubclassOf(cltypeof<PanelWindow>()))
            return;
        m_registeredPanelTypes.push_back(type);

        // Register panel menu entry under Window submenu
        if (m_editor)
        {
            auto editorMenu = MenuManager::GetOrAddMenu(m_editor->GetMenuName());
            auto menu = editorMenu->FindOrNewMenuEntry("Window");

            auto displayName = StringUtil::FriendlyName(type->GetShortName());

            auto checkAction = MenuCheckAction::FromLambda([](MenuContexts_sp ctxs, bool checked) {
                if (auto ctx = ctxs->FindContext<EditorWindowMenuContext>())
                {
                    if (ctx->m_editorWindow)
                    {
                        auto type = AssemblyManager::GlobalFindType(ctxs->EntryName);
                        if (checked)
                        {
                            ctx->m_editorWindow->OpenPanel(type);
                        }
                        else
                        {
                            ctx->m_editorWindow->ClosePanel(type);
                        }
                    }
                }
            });
            auto getCheckAction = MenuGetCheckedAction::FromLambda([](MenuContexts_sp ctxs) {
                if (auto ctx = ctxs->FindContext<EditorWindowMenuContext>())
                {
                    if (ctx->m_editorWindow)
                    {
                        auto type = AssemblyManager::GlobalFindType(ctxs->EntryName);
                        return ctx->m_editorWindow->IsOpenedPanel(type);
                    }
                }
                return false;
            });
            menu->AddEntry(mksptr(new MenuEntryCheck(type->GetName(), displayName, checkAction, getCheckAction)));
        }
    }

    bool EditorWindow::ContainsPanelType(Type* type) const
    {
        return std::ranges::contains(m_registeredPanelTypes, type);
    }

    void EditorWindow::OpenPanel(Type* type)
    {
        if (ContainsPanelType(type))
        {
            auto newWindow = sptr_cast<PanelWindow>(type->CreateSharedInstance({}));
            newWindow->m_parentWindowId = GetWindowId();
            newWindow->m_parentEditorWindow = this;

            m_openedPanels.push_back(newWindow);
            newWindow->SetOpened(true);
            newWindow->OnOpen();
        }
    }
    void EditorWindow::ClosePanel(Type* type)
    {
        if (type == nullptr)
            return;

        for (auto it = m_openedPanels.begin(); it != m_openedPanels.end(); ++it)
        {
            if ((*it)->GetType() == type)
            {
                (*it)->OnClose();
                (*it)->SetOpened(false);
                m_openedPanels.erase(it);
                break;
            }
        }
    }
    bool EditorWindow::IsOpenedPanel(Type* type) const
    {
        if (type == nullptr)
        {
            return false;
        }
        for (auto& panel : m_openedPanels)
        {
            if (panel->GetType() == type)
            {
                return true;
            }
        }
        return false;
    }

    void EditorWindow::OnPanelStateChanged(PanelWindow* win, bool open)
    {
        // change panel menu state
    }
} // namespace pulsared