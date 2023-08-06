#include "Workspace.h"
#include <CoreLib.Platform/Window.h>
#include <CoreLib/File.h>
#include <Pulsar/Logger.h>
#include <filesystem>

namespace pulsared
{
    static string project_path;
    static string project_name;
    static bool m_isOpened = false;

    Action<> Workspace::OnWorkspaceOpened;
    Function<bool> Workspace::OnWorkspaceClosing;
    Action<> Workspace::OnWorkspaceClosed;

    //.pulsar
    class WorkspaceFile
    {

    };

    bool Workspace::OpenDialogUserWorkspace()
    {
        using namespace jxcorlib::platform;

        string selected_path;
        if (window::OpenFileDialog(window::GetMainWindowHandle(), "seproj(*.seproj)|*.seproj", "", &selected_path))
        {
            OpenWorkspace(selected_path);
            return OpenWorkspace(selected_path);
        }
        return false;
    }

    bool Workspace::OpenWorkspace(string_view path)
    {
        if (!std::filesystem::exists(path))
        {
            return false;
        }

        if (m_isOpened)
        {
            if (!CloseWorkspace())
            {
                return false;
            }
        }

        //open
        project_path = path;
        PathUtil::GenericSelf(&project_path);
        project_name = PathUtil::GetFilenameWithoutExt(project_path);

        Logger::Log("open workspace: " + project_path);
        OnWorkspaceOpened.Invoke();
        return true;
    }

    bool Workspace::CloseWorkspace()
    {
        if (OnWorkspaceClosing.IsValidReturnInvoke())
        {
            OnWorkspaceClosed.Invoke();

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

