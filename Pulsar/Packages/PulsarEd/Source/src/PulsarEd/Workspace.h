#pragma once
#include "Assembly.h"
#include <CoreLib/Events.hpp>
#include <filesystem>

namespace pulsared
{
    //ext .peproj
    class ProjectFile : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ProjectFile, Object);

    public:
        virtual string GetEditorType() = 0;

        string EditorType;
    };

    class ProgramPackage
    {
    public:
        string Name;
        std::filesystem::path Path;
    };

    class Workspace final
    {
    public:

        static bool OpenDialogUserWorkspace();
        static bool OpenWorkspace(std::filesystem::path path);
        static bool CloseWorkspace();

        static bool IsOpened();
        static const string& WorkspacePath();
        static const string& WorkspaceName();
        static string LibraryPath();

        static Action<> OnWorkspaceOpened;
        static Function<bool> OnWorkspaceClosing;
        static Action<> OnWorkspaceClosed;
    };


}