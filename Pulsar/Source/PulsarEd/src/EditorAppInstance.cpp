#include "EditorAppInstance.h"
#include "EditorAppInstance.h"
#include <PulsarEd/EditorAppInstance.h>
#include <Pulsar/Application.h>
#include <Pulsar/World.h>
#include <Pulsar/Logger.h>
#include <filesystem>
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/Private/InputInterface.h>
#include <PulsarEd/Windows/EditorWindowManager.h>
#include <PulsarEd/EditorUIConfig.h>
#include <CoreLib/File.h>
#include <CoreLib.Serialization/JsonSerializer.h>
#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuEntrySubMenu.h>
#include <PulsarEd/IEditorTickable.h>
#include <PulsarEd/EditorLogRecorder.h>
#include <PulsarEd/Subsystems/EditorSubsystem.h>
#include <Pulsar/Scene.h>
#include <PulsarEd/EditorWorld.h>
#include <PulsarEd/AssetDatabase.h>
#include <Pulsar/AssetRegistry.h>
#include <gfx/GFXRenderPipeline.h>

#include <gfx/GFXFrameBufferObject.h>

namespace pulsared
{

    static bool _RequestQuit()
    {
        //请求关闭程序
        return Application::inst()->RequestQuitEvents.IsValidReturnInvoke();
    }
    static void _quitting()
    {
        Logger::Log("engine application is quitting");

        //通知程序即将关闭
        Application::inst()->QuittingEvents.Invoke();
    }

    const char* EditorAppInstance::AppType()
    {
        return "editor";
    }
    void EditorAppInstance::RequestQuit()
    {
        //SystemInterface::RequestQuitEvents();
    }
    Vector2f EditorAppInstance::GetOutputScreenSize()
    {
        return this->output_size_;
    }
    void EditorAppInstance::SetOutputScreenSize(Vector2f size)
    {
        this->output_size_ = size;
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
                Logger::Log("pulsar engine");
                });
            main_menu->FindMenuEntry<MenuEntrySubMenu>("Help")->AddEntry(about);
        }
    }


    void EditorAppInstance::OnPreInitialize(gfx::GFXGlobalConfig* config)
    {
        using namespace std::filesystem;
        EditorLogRecorder::Initialize();

        auto uicfg = PathUtil::Combine(AppRootDir(), "uiconfig.json");

        // load config
        if (exists(path{ uicfg }))
        {
            auto json = FileUtil::ReadAllText(uicfg);
            auto cfg = ser::JsonSerializer::Deserialize<EditorUIConfig>(json);

            Vector2f winSize = cfg->WindowSize;

            config->WindowWidth = static_cast<int>(winSize.x);
            config->WindowHeight = static_cast<int>(winSize.y);
        }
        else
        {
            config->WindowHeight = 720;
            config->WindowWidth = 1280;
        }
        
        config->EnableValid = true;

        strcpy(config->ProgramName, "Pulsar");
        strcpy(config->Title, "Pulsar Editor v0.1 - Vulkan1.2");


    }

    void EditorAppInstance::OnInitialized()
    {
        //temp
        {
            //AssetRegisterInfo info;
            //info.path = "Engine/Assets/Shaders/BuiltinRP.DefaultLit";
            //info.getter = [&](guid_t) {
            //    auto shader_src = FileUtil::ReadAllText(R"(D:\Codes\Pulsar\Engine\Assets\pbr.shader)");
            //    return Material::StaticCreate(info.path, Shader::StaticCreate(shader_src));
            //};
            //AssetRegistry::RegisterAsset(guid_t::create_new(), info);
        }

        Logger::Log("initialize gfx application");

        //world
        World::Reset<EditorWorld>();

        auto renderPipeline = new EditorRenderPipeline(World::Current());
        
        Application::GetGfxApp()->SetRenderPipeline(renderPipeline);

        Logger::Log("initialize imgui");
        m_gui = CreateImGui(Application::GetGfxApp());
        m_gui->Initialize();
        
        renderPipeline->ImGuiObject = m_gui;

        AssetDatabase::Initialize();

        InitBasicMenu();

        Logger::Log("initialize world");


        Logger::Log("initialize subsystems");
        //collect subsystem
        for (Type* type : *__PulsarSubsystemRegistry::types())
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
            Logger::Log("initializing subsystem: " + subsystem->GetType()->GetName());
            subsystem->OnInitializing();
        }
        for (auto& subsystem : this->subsystems)
        {
            Logger::Log("initialized subsystem: " + subsystem->GetType()->GetName());
            subsystem->OnInitialized();
        }

        //Workspace::OpenWorkspace(R"(D:\Codes\Pulsar\TestProject)");

        //init window uis
        Logger::Log("initialize editor window manager");
        pulsared::EditorWindowManager::Initialize();

    }

    void EditorAppInstance::OnTerminate()
    {
        m_gui->Terminate();
        m_gui.reset();

        delete Application::GetGfxApp()->GetRenderPipeline();
        Application::GetGfxApp()->SetRenderPipeline(nullptr);

        pulsared::EditorWindowManager::Terminate();

        World::Reset(nullptr);

        //terminate subsystem
        for (auto& subsystem : this->subsystems)
        {
            subsystem->OnTerminate();
        }

        using namespace std::filesystem;

        auto uicfg_path = PathUtil::Combine(AppRootDir(), "uiconfig.json");
        auto cfg = mksptr(new EditorUIConfig);

        cfg->WindowSize = GetAppSize();
        auto json = ser::JsonSerializer::Serialize(cfg.get(), {});
        FileUtil::WriteAllText(uicfg_path, json);

        AssetDatabase::Terminate();
        EditorLogRecorder::Terminate();
    }

    void EditorAppInstance::OnBeginRender(float dt)
    {
        m_gui->NewFrame();

        World::Current()->Tick(dt);

        pulsared::EditorWindowManager::Draw();
        pulsared::EditorTickerManager::Ticker.Invoke(dt);

        m_gui->EndFrame();
    }
    void EditorAppInstance::OnEndRender(float dt)
    {

    }
    bool EditorAppInstance::IsQuit()
    {
        //return SystemInterface::GetIsQuit();
        return false;
    }

    rendering::Pipeline* EditorAppInstance::GetPipeline()
    {
        return nullptr;
    }

    Vector2f pulsared::EditorAppInstance::GetAppSize()
    {
        int32_t w, h;
        Application::GetGfxApp()->GetViewport()->GetSize(&w, &h);
        return Vector2f(w, h);
    }
    void pulsared::EditorAppInstance::SetAppSize(Vector2f size)
    {
        Application::GetGfxApp()->GetViewport()->SetSize(size.x, size.y);
    }

    bool EditorAppInstance::IsInteractiveRendering() const
    {
        return m_isPlaying;
    }

    void EditorAppInstance::StartInteractiveRendering()
    {
        m_isPlaying = true;
    }

    void EditorAppInstance::StopInteractiveRendering()
    {
        m_isPlaying = false;
    }


}