#pragma once
#include "Assembly.h"
#include <CoreLib/Events.hpp>


namespace apatiteed
{
    class Project final
    {
    public:
        static void OpenDialogUserProject();
        static void OpenProject(string_view path);
        static bool CloseProject();

        static const string& ProjectPath();
        static const string& ProjectName();
        static const string& LibraryPath();

        static Action<> OnProjectOpened;
        static Function<bool> OnProjectClosing;
        static Action<> OnProjectClosed;
    };

    
}