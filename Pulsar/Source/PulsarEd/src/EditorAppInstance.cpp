#include "EditorAppInstance.h"
#include "EditorAssetManager.h"
#include "EditorRenderPipeline.h"
#include "Importers/FBXImporter.h"
#include "Pulsar/Components/StaticMeshRendererComponent.h"
#include "Tools/ObjectDebugTool.h"

#include <CoreLib.Serialization/JsonSerializer.h>
#include <CoreLib/File.h>
#include <Pulsar/Application.h>
#include <Pulsar/AssetRegistry.h>
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Scene.h>
#include <Pulsar/World.h>
#include <PulsarEd/AssetDatabase.h>
#include <PulsarEd/EditorAppInstance.h>
#include <PulsarEd/EditorLogRecorder.h>
#include <PulsarEd/EditorUIConfig.h>
#include <PulsarEd/EditorWorld.h>
#include <PulsarEd/IEditorTickable.h>
#include <PulsarEd/Menus/Types.h>
#include <PulsarEd/Subsystems/EditorSubsystem.h>
#include <PulsarEd/Windows/EditorWindowManager.h>
#include <filesystem>
#include <gfx/GFXRenderPipeline.h>

#include <CoreLib.Serialization/DataSerializer.h>
#include <Pulsar/Assets/StaticMesh.h>

#include <PulsarEd/Windows/SceneWindow.h>
#include <PulsarEd/Windows/WorkspaceWindow.h>
#include <PulsarEd/Windows/ConsoleWindow.h>
#include <PulsarEd/Windows/PropertiesWindow.h>
#include <PulsarEd/Windows/OutlinerWindow.h>
#include <PulsarEd/Windows/OutputWindow.h>
#include <PulsarEd/Tools/MenuDebugTool.h>

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
        return {};
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
        auto mainMenu = MenuManager::GetMainMenu();
        {
            MenuEntrySubMenu_sp file = mksptr(new MenuEntrySubMenu("File"));
            mainMenu->AddEntry(file);

            auto openWorkSpace = mksptr(new MenuEntryButton("Open Workspace"));
            openWorkSpace->Action = MenuAction::FromRaw([](sptr<MenuContexts> ctx) {
                Workspace::OpenDialogUserWorkspace();
            });
            file->AddEntry(openWorkSpace);
        }
        {
            MenuEntrySubMenu_sp menu = mksptr(new MenuEntrySubMenu("Edit"));
            mainMenu->AddEntry(menu);
        }
        {
            MenuEntrySubMenu_sp menu = mksptr(new MenuEntrySubMenu("Assets"));
            mainMenu->AddEntry(menu);
        }
        {
            MenuEntrySubMenu_sp menu = mksptr(new MenuEntrySubMenu("Build"));
            mainMenu->AddEntry(menu);
        }
        {
            MenuEntrySubMenu_sp menu = mksptr(new MenuEntrySubMenu("Tool"));
            mainMenu->AddEntry(menu);
            {
                auto entry = mksptr(new MenuEntryButton("MenuDebug"));
                entry->Action = MenuAction::FromRaw([](sptr<MenuContexts> ctx) {
                    ToolWindow::OpenToolWindow<MenuDebugTool>();
                });
                menu->AddEntry(entry);
            }
            {
                auto entry = mksptr(new MenuEntryButton("ObjectDebug"));
                entry->Action = MenuAction::FromRaw([](sptr<MenuContexts> ctx) {
                    ToolWindow::OpenToolWindow<ObjectDebugTool>();
                });
                menu->AddEntry(entry);
            }
        }
        {
            MenuEntrySubMenu_sp menu = mksptr(new MenuEntrySubMenu("Window"));
            mainMenu->AddEntry(menu);
        }
        {
            MenuEntrySubMenu_sp menu = mksptr(new MenuEntrySubMenu("Help"));
            mainMenu->AddEntry(menu);
        }

        {
            MenuEntryButton_sp menu = mksptr(new MenuEntryButton("About"));
            menu->Action = MenuAction::FromRaw([](auto ctx) {
                Logger::Log("pulsar engine");
            });
            mainMenu->FindMenuEntry<MenuEntrySubMenu>("Help")->AddEntry(menu);
        }

    }

    static void _InitWindowMenu()
    {
        EditorWindowManager::RegisterPanelWindowType(cltypeof<SceneWindow>());
        EditorWindowManager::GetPanelWindow(cltypeof<SceneWindow>())->Open();
        //EditorWindowManager::RegisterPanelWindowType(cltypeof<OutputWindow>());
        //EditorWindowManager::GetPanelWindow(cltypeof<OutputWindow>())->Open();
        EditorWindowManager::RegisterPanelWindowType(cltypeof<OutlinerWindow>());
        EditorWindowManager::GetPanelWindow(cltypeof<OutlinerWindow>())->Open();
        EditorWindowManager::RegisterPanelWindowType(cltypeof<PropertiesWindow>());
        EditorWindowManager::GetPanelWindow(cltypeof<PropertiesWindow>())->Open();
        EditorWindowManager::RegisterPanelWindowType(cltypeof<WorkspaceWindow>());
        EditorWindowManager::GetPanelWindow(cltypeof<WorkspaceWindow>())->Open();
        EditorWindowManager::RegisterPanelWindowType(cltypeof<ConsoleWindow>());
        EditorWindowManager::GetPanelWindow(cltypeof<ConsoleWindow>())->Open();

    }

    void EditorAppInstance::OnPreInitialize(gfx::GFXGlobalConfig* config)
    {
        using namespace std::filesystem;
        EditorLogRecorder::Initialize();

        auto uicfg = PathUtil::Combine(AppRootDir(), "uiconfig.json");

        // load config
        if (exists(path{uicfg}))
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

        Logger::Log("pre intialized");
    }

    static void _Test()
    {
        auto meshNode = Node::StaticCreate("mesh test");
        auto meshComponent = meshNode->AddComponent<StaticMeshRendererComponent>();

        auto staticMesh = GetAssetManager()->LoadAsset<StaticMesh>("Engine/Shapes/pSphere1");
        meshComponent->SetStaticMesh(staticMesh);

        auto missingMat = GetAssetManager()->LoadAsset<Material>("Engine/Materials/Missing");
        meshComponent->SetMaterial(0, missingMat);

        World::Current()->GetPersistentScene()->AddNode(meshNode);
    }

    static void _RegisterIcon(Type* type, string_view path)
    {
        auto icon = FileUtil::ReadAllBytes(AssetDatabase::PackagePathToPhysicsPath(path));
        AssetDatabase::IconPool->Register(index_string(type->GetName()), icon.data(), icon.size());
    }

    static void _RegisterIcon(string_view name, string_view path)
    {
        auto icon = FileUtil::ReadAllBytes(AssetDatabase::PackagePathToPhysicsPath(path));
        AssetDatabase::IconPool->Register(index_string(name), icon.data(), icon.size());
    }

    static std::filesystem::path _SearchUpFolder(std::filesystem::path p)
    {
        namespace fs = std::filesystem;

        fs::path curPath = fs::current_path();
        while (!fs::exists(curPath / p))
        {
            curPath = curPath.parent_path();
        }
        return curPath / p;
    }

    void EditorAppInstance::OnInitialized()
    {
        m_assetManager = new EditorAssetManager;

        // search package path
        AssetDatabase::Initialize();
        AssetDatabase::AddProgramPackageSearchPath(_SearchUpFolder("Packages"));

        // add package
        AssetDatabase::AddPackage("Engine");
        AssetDatabase::AddPackage("Editor");

        //world
        Logger::Log("initialize world");
        m_world = World::Reset<EditorWorld>();

        auto renderPipeline = new EditorRenderPipeline(m_world);

        Application::GetGfxApp()->SetRenderPipeline(renderPipeline);

        Logger::Log("initialize imgui");
        m_gui = CreateImGui(Application::GetGfxApp());
        m_gui->Initialize();
        renderPipeline->ImGuiObject = m_gui;

        // TODO: setup layout
        {
            const auto defaultLayoutPath = AssetDatabase::PackagePathToPhysicsPath("Editor/Layout/Default.ini");
            m_gui->SetLayoutInfo(FileUtil::ReadAllText(defaultLayoutPath));
        }

        _RegisterIcon(cltypeof<FolderAsset>(), "Editor/Icons/folder.png");
        _RegisterIcon(cltypeof<Shader>(), "Editor/Icons/shader.png");
        _RegisterIcon(cltypeof<Material>(), "Editor/Icons/material.png");
        _RegisterIcon(cltypeof<StaticMesh>(), "Editor/Icons/staticmesh.png");
        _RegisterIcon(cltypeof<AssetObject>(), "Editor/Icons/PreviewFrame.png");
        _RegisterIcon("WorkspaceWindow.Dirty", "Editor/Icons/Star.png");

        InitBasicMenu();

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

        //init window ui
        Logger::Log("initialize editor window manager");
        EditorWindowManager::Initialize();
        _InitWindowMenu();

        Workspace::OpenWorkspace(_SearchUpFolder("Project") / "Project.peproj");
        _Test();
    }

    void EditorAppInstance::OnTerminate()
    {
        delete Application::GetGfxApp()->GetRenderPipeline();
        Application::GetGfxApp()->SetRenderPipeline(nullptr);

        World::Reset(nullptr);

        //terminate subsystem
        for (auto& subsystem : this->subsystems)
        {
            subsystem->OnTerminate();
        }

        m_gui->Terminate();
        m_gui.reset();
        EditorWindowManager::Terminate();

        using namespace std::filesystem;

        auto uicfg_path = PathUtil::Combine(AppRootDir(), "uiconfig.json");
        auto cfg = mksptr(new EditorUIConfig);

        cfg->WindowSize = GetAppSize();
        auto json = ser::JsonSerializer::Serialize(cfg.get(), {});
        FileUtil::WriteAllText(uicfg_path, json);

        delete m_assetManager;
        m_assetManager = nullptr;
        AssetDatabase::Terminate();
        RuntimeObjectWrapper::Terminate();
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

    Vector2f EditorAppInstance::GetAppSize()
    {
        int32_t w, h;
        Application::GetGfxApp()->GetViewport()->GetSize(&w, &h);
        return Vector2f((float)w, (float)h);
    }

    void EditorAppInstance::SetAppSize(Vector2f size)
    {
        Application::GetGfxApp()->GetViewport()->SetSize((int)size.x, (int)size.y);
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
