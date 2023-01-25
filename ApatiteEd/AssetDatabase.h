#pragma once
#include "Assembly.h"


namespace apatiteed
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
        static void Initialize();
        static void Refresh();
        static void Terminate();
        static inline FileNode FileTree;
    };
}