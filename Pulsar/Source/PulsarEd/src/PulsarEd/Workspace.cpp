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

        fs::path selected_path;

        if (window::OpenFileDialog(window::GetMainWindowHandle(), "peproj(*.peproj);*.peproj", "", &selected_path))
        {
            return OpenWorkspace(selected_path);
        }
        return false;
    }

    bool Workspace::OpenWorkspace(std::filesystem::path path)
    {
        auto absolutePath = std::filesystem::absolute(path);
        if (!std::filesystem::exists(absolutePath))
        {
            Logger::Log("not exists workspace: " + project_path, LogLevel::Warning);
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
        project_path = absolutePath.string();
        PathUtil::GenericSelf(&project_path);
        project_name = PathUtil::GetFilenameWithoutExt(project_path);

        project_path = PathUtil::GetDirectory(project_path);

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

