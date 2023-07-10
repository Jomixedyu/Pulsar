#include "EditorAppInstance.h"
#include "EditorAppInstance.h"
#include <PulsarEd/EditorAppInstance.h>
#include <Pulsar/Application.h>
#include <Pulsar/World.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Private/SystemInterface.h>
#include <Pulsar/Private/RenderInterface.h>
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
#include <ThirdParty/glad/glad.h>
#include <PulsarEd/AssetDatabase.h>
#include <Pulsar/AssetRegistry.h>
#include <gfx/GFXRenderPipeline.h>

namespace pulsared
{
    class RenderPipeline : public gfx::GFXRenderPipeline
    {
        virtual void OnRender(gfx::GFXRenderContext* context, const std::vector<gfx::GFXFrameBufferObject*>& renderTargets)
        {

        }
    };

    using namespace detail;

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
        SystemInterface::RequestQuitEvents();
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

    void EditorAppInstance::OnPreInitialize(gfx::GFXGlobalConfig* config)
    {
        config->EnableValid = true;
        config->WindowWidth = 1280;
        config->WindowHeight = 720;
        strcpy(config->ProgramName, "Pulsar");
        strcpy(config->Title, "Pulsar Editor v0.1 - Vulkan1.3");
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

    void EditorAppInstance::OnInitialized()
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



        //SystemInterface::InitializeWindow(title, (int)size.x, (int)size.y);

        //SystemInterface::SetRequestQuitCallBack(_RequestQuit);
        //SystemInterface::SetQuitCallBack(_quitting);

        //RenderInterface::SetViewport(0, 0, (int)size.x, (int)size.y);

        for (GLenum err; (err = glGetError()) != GL_NO_ERROR;)
        {
            Logger::Log("opengl init error: " + std::to_string(err), LogLevel::Error);
        }

        AssetDatabase::Initialize();

        this->render_pipeline_ = new builtinrp::BultinRP;

        Logger::Log("initialize imgui");
        ImGui_Engine_Initialize();

        InitBasicMenu();

        Logger::Log("initialize world");
        //world
        World::Reset(new EditorWorld);
        World::Current()->scene = mksptr(new Scene);

        //init window uis
        Logger::Log("initialize editor window manager");
        pulsared::EditorWindowManager::Reset();

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

        delete this->render_pipeline_;

        using namespace std::filesystem;

        auto uicfg_path = PathUtil::Combine(AppRootDir(), "uiconfig.json");
        auto cfg = mksptr(new EditorUIConfig);

        cfg->window_size = GetAppSize();
        auto json = ser::JsonSerializer::Serialize(cfg.get(), {});
        FileUtil::WriteAllText(uicfg_path, json);

        AssetDatabase::Terminate();
        EditorLogRecorder::Terminate();
    }

    void EditorAppInstance::OnTick(float dt)
    {

        auto bgc = LinearColorf{ 0.2f, 0.2f ,0.2f, 0.2f };
        RenderInterface::Clear(bgc.r, bgc.g, bgc.b, bgc.a);
        ImGui_Engine_NewFrame();

        World::Current()->Tick(dt);

        pulsared::EditorWindowManager::Draw();
        pulsared::EditorTickerManager::Ticker.Invoke(dt);

        ImGui_Engine_EndFrame();

        RenderInterface::Render();
        SystemInterface::PollEvents();
        InputInterface::PollEvents();

        for (GLenum err; (err = glGetError()) != GL_NO_ERROR;)
        {
            Logger::Log("opengl error: " + std::to_string(err), LogLevel::Error);
        }
    }

    bool EditorAppInstance::IsQuit()
    {
        return SystemInterface::GetIsQuit();
    }

    rendering::Pipeline* EditorAppInstance::GetPipeline()
    {
        return this->render_pipeline_;
    }

    Vector2f pulsared::EditorAppInstance::GetAppSize()
    {
        int32_t w, h;
        RenderInterface::GetDefaultBufferViewport(&w, &h);
        return Vector2f(w, h);
    }
    void pulsared::EditorAppInstance::SetAppSize(Vector2f size)
    {
        RenderInterface::SetViewport(0, 0, (int)size.x, (int)size.y);
    }


}