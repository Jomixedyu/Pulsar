#include "AssetDatabase.h"
#include "Workspace.h"
#include "AssetProviders/AssetProvider.h"
#include <filesystem>

namespace apatiteed
{
    static map<guid_t, string> assets;
    static map<string, AssetProvider*> providers;

    static void _Scan(FileNode* node)
    {
        for (auto& i : std::filesystem::directory_iterator(node->path))
        {
            FileNode new_node;
            new_node.path = i.path().generic_string();
            new_node.name = i.path().filename().string();
            new_node.is_folder = i.is_directory();
            
            if (new_node.is_folder)
            {
                _Scan(&new_node);
            }

            node->children.push_back(new_node);
        }
    }

    static void _OnWorkspaceOpened()
    {
        // initialize asset database data
        FileNode node;
        node.path = Workspace::WorkspacePath();
        node.name = Workspace::WorkspaceName();
        node.is_folder = true;
        _Scan(&node);

        AssetDatabase::FileTree = node;
    }
    AssetObject_sp AssetDatabase::LoadAssetAtPath(string_view path)
    {

        return AssetObject_sp();
    }
    void AssetDatabase::Initialize()
    {
        Workspace::OnWorkspaceOpened += _OnWorkspaceOpened;
    }

    void AssetDatabase::Refresh()
    {
    }

    void AssetDatabase::Terminate()
    {
        Workspace::OnWorkspaceOpened -= _OnWorkspaceOpened;
    }
}