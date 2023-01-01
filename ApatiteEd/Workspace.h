#pragma once
#include "Assembly.h"
#include <CoreLib/Events.hpp>


namespace apatiteed
{
    class Workspace final
    {
    public:
        static void OpenDialogUserWorkspace();
        static void OpenWorkspace(string_view path);
        static bool CloseWorkspace();

        static bool IsOpened();
        static const string& WorkspacePath();
        static const string& WorkspaceName();
        static const string& LibraryPath();

        static Action<> OnWorkspaceOpened;
        static Function<bool> OnWorkspaceClosing;
        static Action<> OnWorkspaceClosed;
    };

    
}