#include "EditorAppInstance.h"
#include <Pulsar/Input.h>
#include "EditorAssetManager.h"
#include "EditorRenderPipeline.h"
#include "Editors/EditorWindow.h"
#include "Editors/EditorRegistry.h"
#include "Editors/SceneEditor/SceneEditor.h"
#include "PulsarEd/UIControls/ViewportFrame.h"
#include "Pulsar/Components/PointLightComponent.h"

#include "Shaders/EditorShaderCompileService.h"
#include "Shaders/ShaderHotReloadWatcher.h"
#include "Utils/PrefabUtil.h"

#include <Pulsar/Rendering/ShaderInstanceCache.h>
#include <Pulsar/Rendering/RenderGraph/TransientRTPool.h>

#include <fstream>
#include <CoreLib.Serialization/JsonSerializer.h>
#include <CoreLib/File.h>
#include <Pulsar/Application.h>
#include <Pulsar/Components/BoxShape3DComponent.h>
#include <Pulsar/Components/DirectionalLightComponent.h>
#include <Pulsar/Components/SphereShape3DComponent.h>
#include <Pulsar/Components/StaticMeshRendererComponent.h>
#include <Pulsar/Components/VolumeComponent.h>
#include <Pulsar/Assets/VolumeProfile.h>
#include <Pulsar/Assets/TonemappingSettings.h>
#include <Pulsar/Assets/DisplayEncodingSettings.h>
#include <Pulsar/ImGuiImpl.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Physics3D/RigidBodyDynamics3DComponent.h>
#include <Pulsar/Prefab.h>
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
#include <random>
#include <utility>

#include <CoreLib.Serialization/DataSerializer.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Assets/StaticMesh.h>
#include <Pulsar/Assets/Texture2D.h>


namespace pulsared
{

    static bool _RequestQuit()
    {
        // 请求关闭程序
        return Application::inst()->RequestQuitEvents.IsValidReturnInvoke();
    }

    static void _quitting()
    {
        Logger::Log("engine application is quitting");

        // 通知程序即将关闭
        Application::inst()->QuittingEvents.Invoke();
    }



    void EditorAppInstance::OnCreateEditors()
    {
    }
    const char* EditorAppInstance::AppType()
    {
        return "editor";
    }

    void EditorAppInstance::RequestQuit()
    {
        m_shouldQuit = true;
    }

    Vector2f EditorAppInstance::GetOutputScreenSize()
    {
        return this->m_outputSize;
    }

    void EditorAppInstance::SetOutputScreenSize(Vector2f size)
    {
        this->m_outputSize = size;
    }

    string EditorAppInstance::GetTitle()
    {
        return {};
    }

    void EditorAppInstance::SetTitle(string_view title)
    {
    }

    std::filesystem::path EditorAppInstance::AppRootDir()
    {
        return std::filesystem::current_path();
    }
    std::filesystem::path EditorAppInstance::GetTempDirectory()
    {
        auto tempDir = AppRootDir() / "Temp";
        std::filesystem::create_directories(tempDir);
        return tempDir;
    }

    namespace ShapeMeshUtils
    {
        ObjectPtr<Node> CreateCube(
            RCPtr<NodeCollection> scene,
            string name = "Node",
            RCPtr<Material> material = nullptr,
            bool dynamicBody = false,
            Vector3f position = Vector3f(),
            Vector3f rotation = Vector3f(),
            Vector3f scale = Vector3f(1.0f, 1.0f, 1.0f))
        {
            if (material == nullptr)
            {
                material = AssetManager::Get()->LoadAsset<Material>("Pulsar/Materials/Lambert", true);
            }
            auto cube = AssetManager::Get()->LoadAsset<StaticMesh>("Pulsar/Shapes/Cube", true);

            auto node = scene->NewNode(name);
            node->AddComponent<BoxShape3DComponent>();
            auto body = node->AddComponent<RigidBodyDynamics3DComponent>();
            if (dynamicBody)
            {
                body->SetMode(RigidBody3DMode::Dynamic);
            }
            auto comp = node->AddComponent<StaticMeshRendererComponent>();
            comp->SetStaticMesh(cube);
            comp->SetMaterial(0, material);
            node->GetTransform()->SetPosition(position);
            node->GetTransform()->SetEuler(rotation);
            node->GetTransform()->SetScale(scale);
            return node;
        }
        ObjectPtr<Node> CreateSphere(
            RCPtr<NodeCollection> scene,
            string name = "Node",
            RCPtr<Material> material = nullptr,
            bool dynamicBody = false,
            Vector3f position = Vector3f(),
            Vector3f rotation = Vector3f(),
            Vector3f scale = Vector3f(1.0f, 1.0f, 1.0f))
        {
            auto sphere = AssetManager::Get()->LoadAsset<StaticMesh>("Pulsar/Shapes/Sphere", true);
            if (material == nullptr)
            {
                material = AssetManager::Get()->LoadAsset<Material>("Pulsar/Materials/Lambert", true);
            }

            auto node = scene->NewNode(name);
            node->AddComponent<SphereShape3DComponent>();
            auto body = node->AddComponent<RigidBodyDynamics3DComponent>();
            if (dynamicBody)
            {
                body->SetMode(RigidBody3DMode::Dynamic);
            }
            auto comp = node->AddComponent<StaticMeshRendererComponent>();
            comp->SetStaticMesh(sphere);
            comp->SetMaterial(0, material);
            node->GetTransform()->SetPosition(position);
            node->GetTransform()->SetEuler(rotation);
            node->GetTransform()->SetScale(scale);
            return node;
        }
    }

    void EditorAppInstance::OnPreInitialize(gfx::GFXGlobalConfig* config)
    {
        using namespace std::filesystem;
        EditorLogRecorder::Initialize();

        auto uicfg = AppRootDir() / "uiconfig.json";

        // load config
        if (exists(uicfg))
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

        StringUtil::strcpy(config->ProgramName, "Pulsar");
        StringUtil::strcpy(config->Title, "Pulsar Editor v0.2 - Vulkan1.3");

        Logger::Log("pre intialized");
    }

    void EditorAppInstance::SetupDefaultResidentScene()
    {

        auto scene = m_world->GetFocusScene();
        if (!scene)
        {
            scene = m_world->GetResidentScene();
        }

        auto cam = m_world->GetCurrentCamera();
        cam->GetTransform()->SetPosition({0,0,-50});
        cam->GetTransform()->GetParent()->SetEuler({});

        // light
        {
            auto dlight = scene->NewNode("Directional Light");
            dlight->AddComponent<DirectionalLightComponent>();
            dlight->GetTransform()->TranslateRotateEuler({-3,3,-3}, {45,45,0});
        }

        // postprocess volume
        {
            auto ppVolume = scene->NewNode("PostProcess Volume");
            auto volComp = ppVolume->AddComponent<VolumeComponent>();
            volComp->SetIsGlobal(true);

            auto profile = NewAssetObject<VolumeProfile>();
            profile->GetEffects()->push_back(mksptr(new TonemappingSettings()));
            profile->GetEffects()->push_back(mksptr(new DisplayEncodingSettings()));
            volComp->SetProfile(profile);
        }


        // sky
        // if (0)
//        {
//            auto skySphere = scene->NewNode("Sky Sphere");
//            skySphere->GetTransform()->SetScale({500,500,500});
//            auto sphere = AssetManager::Get()->LoadAsset<StaticMesh>(BuiltinAsset::Shapes_Sphere);
//            auto renderer = skySphere->AddComponent<StaticMeshRendererComponent>();
//            renderer->SetStaticMesh(sphere);
//            renderer->SetMaterial(0, AssetManager::Get()->LoadAsset<Material>("Pulsar/Materials/SkySphere"));
//        }

        // default scene
        auto cube = AssetManager::Get()->LoadAsset<StaticMesh>("Pulsar/Shapes/Cube", true);
        auto sphere = AssetManager::Get()->LoadAsset<StaticMesh>("Pulsar/Shapes/Sphere", true);
        auto gridMat = AssetManager::Get()->LoadAsset<Material>("Pulsar/Materials/WorldGrid", true);
        auto litMat = AssetManager::Get()->LoadAsset<Material>("Pulsar/Materials/Lambert", true);
        ShapeMeshUtils::CreateCube(scene, "floor", gridMat, false, {0, -0.25f, 0},{}, { 10.f, 0.5f, 10.f});
        return;
        {

            ShapeMeshUtils::CreateCube(scene, "cube", gridMat,true, {-2.f,0.5f,2.f},{0.0f, -35.0f,0.0f});
            ShapeMeshUtils::CreateCube(scene, "cube", gridMat,true, {0.f,0.5f,2.f},{0.0f, -10.0f,0.0f});
            ShapeMeshUtils::CreateCube(scene, "cube", gridMat, true, {-0.45f, 1.5f, 2.f}, {0.0f, -40.0f, 0.0f});
            ShapeMeshUtils::CreateCube(scene, "cube", gridMat, true, {-1.65f, 1.25f, 2.f}, {0.0f, -10.0f, 0.0f}, {0.5f, 0.5f, 0.5f});
            ShapeMeshUtils::CreateSphere(scene, "sphere", gridMat, true, {-0.3f, 5.f, 1.3f});

        }


        {
            auto p1 = scene->NewNode("PointLight");
            auto light = p1->AddComponent<PointLightComponent>();
            light->SetColor(Color4f(1,0.2,0.2));
            light->SetIntensity(14);
            light->GetTransform()->SetPosition({-1.7f,2,0});
        }


         {
             std::default_random_engine e;
             std::uniform_real_distribution<float> dis(0.0, 1.0);

             for (int x = 0; x < 10; ++x)
             {
                 for (int y = 0; y < 10; ++y)
                 {
                     ShapeMeshUtils::CreateSphere(scene, "sphere", litMat, true, Vector3f(x - 5, 5+x, y - 5));
                     ShapeMeshUtils::CreateSphere(scene, "sphere", litMat, true, Vector3f(x - 5, 8+x, y - 5));
                     ShapeMeshUtils::CreateSphere(scene, "sphere", litMat, true, Vector3f(x - 5, 10+x, y - 5));
                     ShapeMeshUtils::CreateSphere(scene, "sphere", litMat, true, Vector3f(x - 5, 12+x, y - 5));
                     ShapeMeshUtils::CreateSphere(scene, "sphere", litMat, true, Vector3f(x - 5, 14+x, y - 5));


                     ShapeMeshUtils::CreateCube(scene, "cube", litMat,true, Vector3f(x - 5, 3+y, y - 5),{dis(e) * 360.f, dis(e) * 360.f,dis(e) * 360.f});
                 }
             }
         }

         return;
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
        while (true)
        {
            fs::path candidate = curPath / p;
            if (fs::exists(candidate) && fs::is_directory(candidate))
            {
                // A valid package root contains subdirectories with
                // CMakeLists.txt (each package has one at its root).
                bool hasValidPackage = false;
                for (const auto& entry : fs::directory_iterator(candidate))
                {
                    if (entry.is_directory() && fs::exists(entry.path() / "CMakeLists.txt"))
                    {
                        hasValidPackage = true;
                        break;
                    }
                }
                if (hasValidPackage)
                    return candidate.make_preferred();
            }
            if (!curPath.has_parent_path() || curPath.parent_path() == curPath)
                break;
            curPath = curPath.parent_path();
        }
        return {};
    }


    static void AutoRegisterAssetIcons()
    {
        for (auto asmObj : AssemblyManager::GetAssemblies())
        {
            for (const auto& type : asmObj->GetTypes())
            {
                if (auto attr = type->GetAttribute<AssetIconAttribute>())
                {
                    _RegisterIcon(type, attr->GetIconPath());
                }
            }
        }
    }
    void EditorAppInstance::OnInitialized()
    {
        pulsar::TransientRTPool::Initialize();

        m_assetManager = new EditorAssetManager;
        OnCreateEditors();

        // search package path
        AssetDatabase::Initialize();
        AssetDatabase::AddProgramPackageSearchPath(_SearchUpFolder("Packages"));

        // add package
        AssetDatabase::AddPackage("Pulsar");
        AssetDatabase::AddPackage("PulsarEd");
        AssetDatabase::AddPackage("Project");

        // 注册 Shader 编译服务
        static EditorShaderCompileService s_shaderCompileService;
        pulsar::ShaderCompileServiceLocator::Register(&s_shaderCompileService);

        // 注册 Pending/Error shader 资产；入口名在运行时从 ShaderConfig 按 passName 查找
        {
            auto LoadBuiltinShader = [](const std::string& assetPath) -> RCPtr<pulsar::Shader>
            {
                return cast<pulsar::Shader>(AssetDatabase::LoadAssetAtPath(assetPath));
            };

            pulsar::ShaderInstanceCache::Instance().Initialize(
                LoadBuiltinShader("Pulsar/Shaders/Pending"),
                LoadBuiltinShader("Pulsar/Shaders/Error"));

            Logger::Log("ShaderInstanceCache initialized");
        }


        // world
        Logger::Log("initialize world");
        auto edWorld = new EditorWorld("MainWorld");
        edWorld->OnWorldBegin();
        edWorld->GetCurrentCamera()->GetTransform()->GetParent()->SetEuler({45.f,-45,0});

        m_world = edWorld;

        auto renderPipeline = new EditorRenderPipeline{m_world};

        Application::GetGfxApp()->SetRenderPipeline(renderPipeline);

        Logger::Log("initialize imgui");
        m_gui = CreateImGui(Application::GetGfxApp());
        m_gui->Initialize();
        renderPipeline->ImGuiObject = m_gui;

        // TODO: setup layout
        {
            const auto defaultLayoutPath = AssetDatabase::PackagePathToPhysicsPath("PulsarEd/Layout/Default.ini");
            m_gui->SetLayoutInfo(FileUtil::ReadAllText(defaultLayoutPath));
        }

        AutoRegisterAssetIcons();
        _RegisterIcon("WorkspaceWindow.Dirty", "PulsarEd/Icons/Star.png");


        Logger::Log("initialize subsystems");

        // init window ui
        Logger::Log("initialize editor window manager");
        EditorWindowManager::Initialize();

        EditorRegistry::Initialize();
        if (auto* sceneEditor = EditorRegistry::FindEditor<SceneEditor>())
        {
            sceneEditor->CreateEditorWindow()->Open();
        }

        Workspace::OpenWorkspace(_SearchUpFolder("Packages") / "Project" / "Project.peproj");

        // Create initial editing scene as focus scene
        {
            auto world = GetEditorWorld();
            auto scene = Scene::StaticCreate("NewScene");
            scene->SetObjectFlags(scene->GetObjectFlags() & ~OF_Transient);
            world->LoadScene(scene);
            world->SetFocusScene(scene);
        }

        SetupDefaultResidentScene();

        uinput::InputManager::GetInstance()->Initialize();

        m_shaderHotReloadWatcher = new ShaderHotReloadWatcher();
        m_shaderHotReloadWatcher->Initialize();

    }

    void EditorAppInstance::OnTerminate()
    {
        uinput::InputManager::GetInstance()->Terminate();

        EditorRegistry::Terminate();
        PrefabUtil::ClosePrefabMode();

        if (m_shaderHotReloadWatcher)
        {
            m_shaderHotReloadWatcher->Terminate();
            delete m_shaderHotReloadWatcher;
            m_shaderHotReloadWatcher = nullptr;
        }

        // 在 World 和 GFX Device 销毁前清理 ShaderInstanceCache，
        // 否则 GpuProgram 析构时 VkDevice 已经无效
        pulsar::ShaderInstanceCache::Instance().Clear();
        pulsar::TransientRTPool::Shutdown();

        if (m_world)
        {
            m_world->OnWorldEnd();
            delete m_world;
            m_world = nullptr;
        }

        m_gui->Terminate();
        m_gui.reset();
        EditorWindowManager::Terminate();

        using namespace std::filesystem;

        auto uicfg_path = AppRootDir() / "uiconfig.json";
        auto cfg = mksptr(new EditorUIConfig);

        cfg->WindowSize = GetAppSize();
        auto json = ser::JsonSerializer::Serialize(cfg.get(), {});
        FileUtil::WriteAllText(uicfg_path, json);

        delete Application::GetGfxApp()->GetRenderPipeline();
        Application::GetGfxApp()->SetRenderPipeline(nullptr);

        BuiltinAsset::ClearAssets();

        delete m_assetManager;
        m_assetManager = nullptr;
        AssetDatabase::Terminate();
        RuntimeObjectManager::Terminate();
        EditorLogRecorder::Terminate();
    }

    void EditorAppInstance::TickWorld(float dt)
    {
        // Snapshot world list to avoid iterator invalidation if Tick() mutates gWorlds
        const auto& allWorlds = World::GetAllWorlds();
        array_list<World*> worlds(allWorlds.begin(), allWorlds.end());

        // 1. All worlds snapshot their input state
        for (auto* world : worlds)
            world->BeginInputFrame();

        // 2. Fetch global events once
        auto events = uinput::InputManager::GetInstance()->PollEvents();

        // 3. Let each editor route input to its own panels
        for (auto& editor : EditorRegistry::GetEditors())
        {
            editor->RouteInput(events);
        }

        // 4. Tick all worlds unconditionally
        for (auto* world : worlds)
            world->Tick(dt);
    }

    void EditorAppInstance::OnBeginRender(float dt)
    {
        // 刷新异步 shader 编译回调（主线程）
        if (auto* compileService = dynamic_cast<EditorShaderCompileService*>(pulsar::ShaderCompileServiceLocator::Get()))
        {
            compileService->FlushCallbacks();
        }

        if (m_shaderHotReloadWatcher)
        {
            m_shaderHotReloadWatcher->Tick(dt);
        }

        if (!m_gui->IsMinimized())
        {
            m_gui->NewFrame();

            pulsared::EditorWindowManager::Draw(dt);
            TickWorld(dt);
            pulsared::EditorTickerManager::Ticker.Invoke(dt);

            if (m_modalDialog)
            {
                m_modalDialog->Tick(dt);
                if (m_modalDialog->m_shouldClose)
                    m_modalDialog.reset();
            }

            OnRenderTick.Invoke(dt);

            m_gui->EndFrame();
        }
        else
        {
            TickWorld(dt);
            pulsared::EditorTickerManager::Ticker.Invoke(dt);
        }
    }

    void EditorAppInstance::OnEndRender(float dt)
    {
        RuntimeObjectManager::TickGCollect();
    }

    bool EditorAppInstance::IsQuit()
    {
        // return SystemInterface::GetIsQuit();
        return m_shouldQuit;
    }


    Vector2f EditorAppInstance::GetAppSize()
    {
        int32_t w, h;
        Application::GetGfxApp()->GetWindow()->GetWindowSize(&w, &h);
        return {(float)w, (float)h};
    }

    void EditorAppInstance::SetAppSize(Vector2f size)
    {
        Application::GetGfxApp()->GetWindow()->SetWindowSize((int)size.x, (int)size.y);
    }

    void EditorAppInstance::ShowModalDialog(SPtr<ModalDialog> dialog)
    {
        m_modalDialog = std::move(dialog);
    }


}
