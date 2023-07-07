#include "EngineAppInstance.h"
#include <Pulsar/EngineAppInstance.h>
#include <Pulsar/Application.h>
#include <Pulsar/World.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Private/SystemInterface.h>
#include <Pulsar/Private/RenderInterface.h>
#include <filesystem>
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/Private/InputInterface.h>

namespace pulsar
{
    using namespace detail;
    const char* EngineAppInstance::AppType()
    {
        return "engine";
    }

    static bool _RequestQuit()
    {
        //请求关闭程序
        return Application::inst()->RequestQuitEvents.IsValidReturnInvoke();
    }

    static void _quitting()
    {
        Logger::Log ("engine application is quitting");

        //通知程序即将关闭
        Application::inst()->QuittingEvents.Invoke();
    }

    void EngineAppInstance::OnInitialize(string_view title, Vector2f size)
    {
        Logger::Log("application initialize");
        SystemInterface::InitializeWindow(title, (int)size.x, (int)size.y);
        SystemInterface::SetRequestQuitCallBack(_RequestQuit);
        SystemInterface::SetQuitCallBack(_quitting);
        RenderInterface::SetViewport(0, 0, (int)size.x, (int)size.y);
        ImGui_Engine_Initialize();
        World::Reset(new World);
    }

    void EngineAppInstance::OnTerminate()
    {
        ImGui_Engine_Terminate();
        World::Reset(nullptr);
    }

    void EngineAppInstance::OnTick(float dt)
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
    bool EngineAppInstance::IsQuit()
    {
        return SystemInterface::GetIsQuit();
    }
    void EngineAppInstance::RequestQuit()
    {
        SystemInterface::RequestQuitEvents();
    }

    Vector2f EngineAppInstance::GetOutputScreenSize()
    {
        int32_t x, y;
        detail::RenderInterface::GetDefaultBufferViewport(&x, &y);
        return Vector2f(x, y);
    }
    void EngineAppInstance::SetOutputScreenSize(Vector2f size)
    {
        detail::RenderInterface::SetViewport(0, 0, (int)size.x, (int)size.y);
    }

    string EngineAppInstance::GetTitle()
    {
        return string();
    }
    void EngineAppInstance::SetTitle(string_view title)
    {
    }
    string EngineAppInstance::AppRootDir()
    {
        return StringUtil::StringCast(std::filesystem::current_path().generic_u8string());
    }

}