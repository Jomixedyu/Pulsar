#include "Project.h"
#include <CoreLib.Platform/Window.h>

namespace apatiteed
{
    static string project_path;
    static string project_name;
    static bool is_opened = true;

    //.seproj
    class ProjectFile
    {

    };

    void Project::OpenDialogUserProject()
    {
        using namespace jxcorlib::platform;

        string selected_path;
        if (window::OpenFileDialog(window::MainWindow(), "seproj(*.seproj)|*.seproj", "", &selected_path))
        {
            OpenProject(selected_path);
        }
    }

    void Project::OpenProject(string_view path)
    {
        if (is_opened)
        {
            if (!CloseProject())
            {
                return;
            }
        }

        //open

        project_path = path;
        OnProjectOpened.Invoke();
    }

    bool Project::CloseProject()
    {
        return false;
    }

    string const& Project::ProjectPath()
    {
        return project_path;
    }
    string const& Project::ProjectName()
    {
        return project_name;
    }
    const string& Project::LibraryPath()
    {
        return project_path + "/" + "Library";
    }
}

