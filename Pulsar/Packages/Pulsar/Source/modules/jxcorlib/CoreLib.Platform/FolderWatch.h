#pragma once
#include <string_view>
#include <string>

namespace jxcorlib::platform
{
    enum class FileActionType 
    {
        Add,
        Remove,
        Modify,
        RenameOld,
        RenameNew,
    };

    class FolderWatch
    {
    public:
        FolderWatch(std::string_view path, bool recursive);
        ~FolderWatch();
        void Tick();
    protected:
        std::string path_;
        bool recursive_;
        void* handle_;
    };
}