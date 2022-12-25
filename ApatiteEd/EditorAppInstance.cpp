#include <ApatiteEd/EditorAppInstance.h>
#include <Apatite/Application.h>
#include <Apatite/World.h>
#include <Apatite/Logger.h>
#include <Apatite/Private/SystemInterface.h>
#include <Apatite/Private/RenderInterface.h>
#include <filesystem>
#include <Apatite/ImGuiImpl.h>
#include <Apatite/Private/InputInterface.h>

namespace apatiteed
{
    using namespace detail;

    static bool _RequestQuit()
    {
        //请求关闭程序
        return Application::inst()->RequestQuitEvents.IsValidReturnInvoke();
    }
    static void _quitting()
    {
        Logger::Info("engine application is quitting");

        //通知程序即将关闭
        Application::inst()->QuittingEvents.Invoke();
    }

    const char* EditorAppInstance::AppType()
    {
        return "editor";
    }
    void EditorAppInstance::RequestQuit()
    {
        SystemInterface::RequestQuitEvents();
    }
    Vector2f EditorAppInstance::ScreenSize()
    {
        return Vector2f();
    }
    void EditorAppInstance::SetScreenSize(Vector2f size)
    {
        RenderInterface::SetViewport(0, 0, (int)size.x, (int)size.y);
    }
    string EditorAppInstance::GetTitle()
    {
        return string();
    }
    void EditorAppInstance::SetTitle(string_view title)
    {
    }

    const string& EditorAppInstance::AppRootDir()
    {
        return StringUtil::StringCast(std::filesystem::current_path().generic_u8string());
    }
    void EditorAppInstance::OnInitialize(string_view title, Vector2f size)
    {
        Logger::Info("application initialize");

        SystemInterface::InitializeWindow(title, (int)size.x, (int)size.y);

        SystemInterface::SetRequestQuitCallBack(_RequestQuit);
        SystemInterface::SetQuitCallBack(_quitting);

        RenderInterface::SetViewport(0, 0, (int)size.x, (int)size.y);

        ImGui_Engine_Initialize();

        World::Reset(new World);
    }
    void EditorAppInstance::OnTerminate()
    {
        ImGui_Engine_Terminate();
        World::Reset(nullptr);
    }
    void EditorAppInstance::OnTick(float dt)
    {
        auto bgc = LinearColorf{ 0.2f, 0.2f ,0.2f, 0.2 };
        RenderInterface::Clear(bgc.r, bgc.g, bgc.b, bgc.a);
        ImGui_Engine_NewFrame();

        World::Current()->Tick(dt);

        ImGui_Engine_EndFrame();

        RenderInterface::Render();
        SystemInterface::PollEvents();
        InputInterface::PollEvents();
    }
    bool EditorAppInstance::IsQuit()
    {
        return SystemInterface::GetIsQuit();
    }
}