#include "Menus/Menu.h"
#include "Menus/MenuRenderer.h"

#include <PulsarEd/AssetDatabase.h>
#include <PulsarEd/Windows/AssetEditorWindow.h>

namespace pulsared
{
    namespace
    {
        struct MenuInit
        {
            static void _InitFile(MenuEntrySubMenu_sp menu)
            {
                {
                    auto entry = mksptr(new MenuEntryButton("OpenAsset", "Open Asset"));

                    menu->AddEntry(entry);
                }

                {
                    auto entry = mksptr(new MenuEntryButton("Save", "Save"));
                    entry->Action = MenuAction::FromLambda([](MenuContexts_sp ctxs) {
                        {
                            auto ctx = ctxs->FindContext<AssetEditorMenuContext>();
                            if (!ctx) return;
                            AssetDatabase::Save(ctx->Asset);
                        } });
                    menu->AddEntry(entry);
                }
            }
            MenuInit()
            {
                auto ae = MenuManager::GetOrAddMenu("AssetEditor");
                {
                    auto file = ae->FindOrNewMenuEntry("File");
                    _InitFile(file);
                }
            }
        };
    } // namespace
    ImGuiWindowFlags AssetEditorWindow::GetGuiWindowFlags() const
    {
        return ImGuiWindowFlags_MenuBar 
            | (IsDirtyAsset() ? ImGuiWindowFlags_UnsavedDocument : ImGuiWindowFlags_None)
            | ImGuiWindowFlags_NoCollapse;
    }
    // namespace
    AssetEditorWindow::AssetEditorWindow()
    {
        static MenuInit _init;
        m_winSize = {800, 420};
        m_menuBarCtxs = mksptr(new MenuContexts);
    }
    bool AssetEditorWindow::IsDirtyAsset() const
    {
        if (m_assetObject)
        {
            return AssetDatabase::IsDirty(m_assetObject);
        }
        return false;
    }

    string AssetEditorWindow::GetWindowName() const
    {
        return std::to_string(GetWindowId());
    }
    void AssetEditorWindow::OnDrawImGui()
    {
        base::OnDrawImGui();
        if (ImGui::BeginMenuBar())
        {
            OnRefreshMenuContexts();
            MenuRenderer::RenderMenu(MenuManager::GetMenu("AssetEditor").get(), m_menuBarCtxs);
            ImGui::EndMenuBar();
        }
    }
    void AssetEditorWindow::OnRefreshMenuContexts()
    {
        m_menuBarCtxs->Contexts.clear();
    }
} // namespace pulsared