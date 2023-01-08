#include "EditorAppInstance.h"
#include <ApatiteEd/EditorAppInstance.h>
#include <Apatite/Application.h>
#include <Apatite/World.h>
#include <Apatite/Logger.h>
#include <Apatite/Private/SystemInterface.h>
#include <Apatite/Private/RenderInterface.h>
#include <filesystem>
#include <Apatite/ImGuiImpl.h>
#include <Apatite/Private/InputInterface.h>
#include <ApatiteEd/Windows/EditorWindowManager.h>
#include <ApatiteEd/EditorUIConfig.h>
#include <CoreLib/File.h>
#include <CoreLib.Serialization/JsonSerializer.h>
#include <ApatiteEd/Menus/Menu.h>
#include <ApatiteEd/Menus/MenuEntrySubMenu.h>
#include <ApatiteEd/IEditorTickable.h>
#include <ApatiteEd/EditorLogRecorder.h>
#include <ApatiteEd/Subsystems/EditorSubsystem.h>
#include <Apatite/Scene.h>
#include <ApatiteEd/EditorWorld.h>

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
        Logger::Log(LogLevel::Info, "engine application is quitting");

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

    string EditorAppInstance::AppRootDir()
    {
        return StringUtil::StringCast(std::filesystem::current_path().generic_u8string());
    }

    static void InitBasicMenu()
    {
        auto main_menu = MenuManager::GetMainMenu();
        {
            MenuEntrySubMenu_sp file = mksptr(new MenuEntrySubMenu("File"));
            main_menu->AddEntry(file);
        }
        {
            MenuEntrySubMenu_sp file = mksptr(new MenuEntrySubMenu("Edit"));
            main_menu->AddEntry(file);
        }
        {
            MenuEntrySubMenu_sp file = mksptr(new MenuEntrySubMenu("Assets"));
            main_menu->AddEntry(file);
        }
        {
            MenuEntrySubMenu_sp file = mksptr(new MenuEntrySubMenu("Build"));
            main_menu->AddEntry(file);
        }
        {
            MenuEntrySubMenu_sp file = mksptr(new MenuEntrySubMenu("Tool"));
            main_menu->AddEntry(file);
        }
        {
            MenuEntrySubMenu_sp file = mksptr(new MenuEntrySubMenu("Window"));
            main_menu->AddEntry(file);
        }
        {
            MenuEntrySubMenu_sp file = mksptr(new MenuEntrySubMenu("Help"));
            main_menu->AddEntry(file);
        }

        {
            MenuEntryButton_sp about = mksptr(new MenuEntryButton("About"));
            about->action = MenuAction::FromRaw([](auto ctx) {
                Logger::Log(LogLevel::Info, "apatite engine");
                });
            main_menu->FindMenuEntry<MenuEntrySubMenu>("Help")->AddEntry(about);
        }
    }

    void EditorAppInstance::OnInitialize(string_view title, Vector2f size)
    {
        using namespace std::filesystem;
        EditorLogRecorder::Initialize();

        auto uicfg = PathUtil::Combine(AppRootDir(), "uiconfig.json");
        if (exists(path{ uicfg }))
        {
            auto json = FileUtil::ReadAllText(uicfg);
            auto cfg = ser::JsonSerializer::Deserialize<EditorUIConfig>(json);

            size = cfg->window_size;
        }

        Logger::Log(LogLevel::Info, "initialize application");

        SystemInterface::InitializeWindow(title, (int)size.x, (int)size.y);

        SystemInterface::SetRequestQuitCallBack(_RequestQuit);
        SystemInterface::SetQuitCallBack(_quitting);

        RenderInterface::SetViewport(0, 0, (int)size.x, (int)size.y);

        Logger::Log(LogLevel::Info, "initialize imgui");
        ImGui_Engine_Initialize();

        InitBasicMenu();


        Logger::Log(LogLevel::Info, "initialize world");
        //world
        World::Reset(new EditorWorld);
        World::Current()->scene = mksptr(new Scene);

        //init window uis
        Logger::Log(LogLevel::Info, "initialize editor window manager");
        apatiteed::EditorWindowManager::Reset();

        Logger::Log(LogLevel::Info, "initialize subsystems");
        //collect subsystem
        for (Type* type : *__ApatiteSubsystemRegistry::types())
        {
            if (type->IsSubclassOf(cltypeof<EditorSubsystem>()))
            {
                sptr<Subsystem> subsys = sptr_cast<Subsystem>(type->CreateSharedInstance({}));
                this->subsystems.push_back(subsys);
            }
        }

        //initialize subsystem
        for (auto& subsystem : this->subsystems)
        {
            Logger::Log(LogLevel::Info, "initializing subsystem: " + subsystem->GetType()->get_name());
            subsystem->OnInitializing();
        }
        for (auto& subsystem : this->subsystems)
        {
            Logger::Log(LogLevel::Info, "initialized subsystem: " + subsystem->GetType()->get_name());
            subsystem->OnInitialized();
        }

        Workspace::OpenWorkspace(R"(D:\Codes\Apatite\out\build\x64-Debug)");
    }

    void EditorAppInstance::OnTerminate()
    {
        ImGui_Engine_Terminate();
        World::Reset(nullptr);


        //terminate subsystem
        for (auto& subsystem : this->subsystems)
        {
            subsystem->OnTerminate();
        }

        using namespace std::filesystem;

        auto uicfg_path = PathUtil::Combine(AppRootDir(), "uiconfig.json");
        auto cfg = mksptr(new EditorUIConfig);

        cfg->window_size = GetAppSize();
        auto json = ser::JsonSerializer::Serialize(cfg.get(), {});
        FileUtil::WriteAllText(uicfg_path, json);

        EditorLogRecorder::Terminate();
    }

    void EditorAppInstance::OnTick(float dt)
    {
        auto bgc = LinearColorf{ 0.2f, 0.2f ,0.2f, 0.2f };
        RenderInterface::Clear(bgc.r, bgc.g, bgc.b, bgc.a);
        ImGui_Engine_NewFrame();

        World::Current()->Tick(dt);

        apatiteed::EditorWindowManager::Draw();
        apatiteed::EditorTickerManager::Ticker.Invoke(dt);

        ImGui_Engine_EndFrame();

        RenderInterface::Render();
        SystemInterface::PollEvents();
        InputInterface::PollEvents();
    }

    bool EditorAppInstance::IsQuit()
    {
        return SystemInterface::GetIsQuit();
    }

    Vector2f apatiteed::EditorAppInstance::GetAppSize()
    {
        int32_t w, h;
        RenderInterface::GetViewport(&w, &h);
        return Vector2f(w, h);
    }
    void apatiteed::EditorAppInstance::SetAppSize(Vector2f size)
    {
        RenderInterface::SetViewport(0, 0, (int)size.x, (int)size.y);
    }


}