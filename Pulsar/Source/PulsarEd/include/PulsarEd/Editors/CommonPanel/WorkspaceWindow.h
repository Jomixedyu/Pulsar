#pragma once

#include <optional>
#include "Windows/PanelWindow.h"
#include <PulsarEd/AssetDatabase.h>
#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuContext.h>

namespace pulsared
{
    class AssetsMenuContext : public MenuContextBase
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::AssetsMenuContext, MenuContextBase);
    public:
        explicit AssetsMenuContext(const string& currentPath)
            : CurrentPath(currentPath)
        {
        }
        string CurrentPath;
        array_list< std::weak_ptr<AssetFileNode> > SelectedFiles;
    };
    CORELIB_DECL_SHORTSPTR(AssetsMenuContext);


    class WorkspaceWindow : public PanelWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::WorkspaceWindow, PanelWindow);
    public:
        static string_view StaticWindowName() { return ICON_FK_DATABASE " Workspace###"  "Workspace"; }
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }
        virtual ImGuiWindowFlags GetGuiWindowFlags() const override;

        virtual void OnOpen() override;
        virtual void OnClose() override;
        virtual void OnDrawImGui(float dt) override;

        WorkspaceWindow();

    public:
        void ClearSelectedFile();
        void OnCurrentFolderChanged();
        string GetCurrentFolder() const { return m_currentFolder; }
        void SetCurrentFolder(string_view path);
        void OpenExplorer() const;
        std::filesystem::path GetCurrentPhysicsFolder() const;
    private:
        void OnDrawBar();
        void OnDrawFolderTree();
        void OnDrawContent();
        void RenderFolderTree(SPtr<AssetFileNode> node);
        void RenderFileContent();
        MenuContextBase_sp MakeMenuContext();
    private:
        void OnClick_Import();
    private:
        SPtr<OnGetContext> m_onGetContextCallback;
        string m_currentFolder;

        array_list< std::weak_ptr<AssetFileNode> > m_selectedFiles;

        char search_buf[256];
        float m_iconSize = 60;
        std::optional<float> m_layoutColumnOffset;

        std::shared_ptr<struct AssetTypeTreeNode> m_rootAddAssetContextTreeNode;
    };
}

