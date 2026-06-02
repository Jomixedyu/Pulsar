#include "Menus/Menu.h"
#include "Menus/MenuRenderer.h"

#include <AssetDatabase.h>
#include <Editors/AssetEditor/AssetEditorWindow.h>

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
        return ImGuiWindowFlags_MenuBar | (IsDirtyAsset() ? ImGuiWindowFlags_UnsavedDocument : ImGuiWindowFlags_None) | ImGuiWindowFlags_NoCollapse;
    }
    string_view AssetEditorWindow::GetWindowDisplayName() const
    {
        if (m_assetObject)
        {
            return m_assetObject->GetName();
        }
        return base::GetWindowDisplayName();
    }
    // namespace
    AssetEditorWindow::AssetEditorWindow()
    {
        static MenuInit _init;
        m_winSize = {800, 420};
        m_menuBarCtxs = mksptr(new MenuContexts);
        m_useDockspace = false;
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
    void AssetEditorWindow::OnDrawAssetEditor(float dt)
    {
    }
    void AssetEditorWindow::OnDrawImGui(float dt)
    {
        base::OnDrawImGui(dt);
        OnDrawAssetEditor(dt);
    }
    void AssetEditorWindow::OnBuildMenuContexts(SPtr<MenuContexts> ctxs)
    {
        OnRefreshMenuContexts();
        for (auto& c : m_menuBarCtxs->Contexts)
            ctxs->Contexts.push_back(c);
    }
    void AssetEditorWindow::OnRefreshMenuContexts()
    {
        m_menuBarCtxs->Contexts.clear();
    }
} // namespace pulsared