#pragma once
#include "Assembly.h"


namespace pulsared
{
    struct FileNode
    {
    public:
        string path;
        string name;
        bool is_folder;
        array_list<struct FileNode> children;
    };

    class AssetDatabase
    {
    public:
        static AssetObject_sp LoadAssetAtPath(string_view path);
        static void Initialize();
        static void Refresh();
        static void Terminate();
        static inline FileNode FileTree;
    };
}