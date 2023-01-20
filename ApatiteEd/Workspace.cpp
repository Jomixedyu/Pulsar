#include "Workspace.h"
#include <CoreLib.Platform/Window.h>
#include <CoreLib/File.h>
#include <Apatite/Logger.h>

namespace apatiteed
{
    static string project_path;
    static string project_name;
    static bool is_opened = true;

    Action<> Workspace::OnWorkspaceOpened;
    Function<bool> Workspace::OnWorkspaceClosing;
    Action<> Workspace::OnWorkspaceClosed;

    //.seproj
    class WorkspaceFile
    {

    };

    void Workspace::OpenDialogUserWorkspace()
    {
        using namespace jxcorlib::platform;

        string selected_path;
        if (window::OpenFileDialog(window::MainWindow(), "seproj(*.seproj)|*.seproj", "", &selected_path))
        {
            OpenWorkspace(selected_path);
        }
    }

    void Workspace::OpenWorkspace(string_view path)
    {
        if (is_opened)
        {
            if (!CloseWorkspace())
            {
                return;
            }
        }

        //open

        project_path = path;
        project_name = PathUtil::GetFilenameWithoutExt(project_path);

        Logger::Log("open workspace: " + project_path);
        OnWorkspaceOpened.Invoke();
    }

    bool Workspace::CloseWorkspace()
    {
        if (OnWorkspaceClosing.IsValidReturnInvoke())
        {
            OnWorkspaceOpened.Invoke();

            project_path.clear();
            project_name.clear();
            return true;
        }
        return false;
    }

    bool Workspace::IsOpened()
    {
        return !project_path.empty();
    }

    string const& Workspace::WorkspacePath()
    {
        return project_path;
    }
    string const& Workspace::WorkspaceName()
    {
        return project_name;
    }
    string Workspace::LibraryPath()
    {
        return PathUtil::Combine(project_path, "Library");
    }
}

