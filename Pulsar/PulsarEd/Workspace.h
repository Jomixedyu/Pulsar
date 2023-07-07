#pragma once
#include "Assembly.h"
#include <CoreLib/Events.hpp>


namespace pulsared
{
    class Workspace final
    {
    public:
        static bool OpenDialogUserWorkspace();
        static bool OpenWorkspace(string_view path);
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