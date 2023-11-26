#pragma once
#include "PanelWindow.h"
#include <PulsarEd/AssetDatabase.h>
#include <optional>
#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuContext.h>
#include <CoreLib/index_string.hpp>

namespace pulsared
{
    class WorkspaceWindowMenuContext : public MenuContextBase
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::WorkspaceWindowMenuContext, MenuContextBase);
    public:
        explicit WorkspaceWindowMenuContext(const string& currentPath)
            : CurrentPath(currentPath)
        {
        }
        string CurrentPath;
        array_list< std::weak_ptr<AssetFileNode> > SelectedFiles;
    };
    CORELIB_DECL_SHORTSPTR(WorkspaceWindowMenuContext);


    class WorkspaceWindow : public PanelWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::WorkspaceWindow, PanelWindow);
    public:
        static string_view StaticWindowName() { return ICON_FK_DATABASE " Workspace###"  "Workspace"; }
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }
        virtual ImGuiWindowFlags GetGuiWindowFlags() const override{
            return ImGuiWindowFlags_None | ImGuiWindowFlags_MenuBar;
        }

        virtual void OnOpen() override;
        virtual void OnClose() override;
        virtual void OnDrawImGui() override;

        WorkspaceWindow();

    public:
        void ClearSelectedFile();
        void OnCurrentFolderChanged();
        string GetCurrentFolder() const { return m_currentFolder; }
        void SetCurrentFolder(string_view path);
    private:
        void OnDrawBar();
        void OnDrawFolderTree();
        void OnDrawContent();
        void RenderFolderTree(sptr<AssetFileNode> node);
        void RenderFileContent();
        WorkspaceWindowMenuContext_sp MakeMenuContext();
    private:
        void OnClick_Import();
    private:
        string m_currentFolder;

        array_list< std::weak_ptr<AssetFileNode> > m_selectedFiles;

        char search_buf[256];
        float m_iconSize = 60;
        std::optional<float> m_layoutColumnOffset;

        std::shared_ptr<struct AssetTypeTreeNode> m_rootAddAssetContextTreeNode;
    };
}

