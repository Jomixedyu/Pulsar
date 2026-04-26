#include "Editors/EditorWindow.h"

#include "Editors/Editor.h"
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
        EditorWindowManager::OnWindowStateChanged.AddListener(this, &EditorWindow::OnGlobalPanelStateChanged);
    }
    void EditorWindow::OnClose()
    {
        base::OnClose();
        EditorWindowManager::OnWindowStateChanged.RemoveListener(this, &EditorWindow::OnGlobalPanelStateChanged);
    }

    void EditorWindow::OnDrawImGui(float dt)
    {
        base::OnDrawImGui(dt);

        // render menu
        if (auto editor = GetEditor())
        {
            auto menu = MenuManager::GetMenu(GetEditor()->GetMenuName());
            auto ctxs = mksptr(new MenuContexts);

            auto ctx = mksptr(new EditorWindowMenuContext);
            ctx->m_editorWindow = this;

            ctxs->Contexts.push_back(ctx);

            if (ImGui::BeginMenuBar())
            {
                MenuRenderer::RenderMenu(menu, ctxs);
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
    }

    void EditorWindow::OpenPanel(Type* type)
    {
        if (auto editor = GetEditor())
        {
            if (editor->ContainsPanelType(type))
            {
                auto newWindow = sptr_cast<PanelWindow>(type->CreateSharedInstance({}));
                newWindow->m_parentWindowId = GetWindowId();

                m_openedPanels.push_back(newWindow);
                newWindow->Open();
            }
        }

    }
    void EditorWindow::ClosePanel(Type* type)
    {
        if (type == nullptr)
        {
            return;
        }

        for (auto& panel : m_openedPanels)
        {
            if (panel->GetType() == type)
            {
                panel->Close();
                std::erase(m_openedPanels, panel);
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

    void EditorWindow::OnGlobalPanelStateChanged(EdGuiWindow* win, bool opened)
    {
        if (win)
        {
            if (win->GetType()->IsSubclassOf(cltypeof<PanelWindow>()))
            {
                auto panel = static_cast<PanelWindow*>(win);
                if (panel->GetParentWindowId() == GetWindowId())
                {
                    OnPanelStateChanged(panel, opened);
                }
            }
        }
    }


    void EditorWindow::OnPanelStateChanged(PanelWindow* win, bool open)
    {
        // change panel menu state


    }
} // namespace pulsared