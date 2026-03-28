#include "EditorAppInstance.h"
#include "EditorAssetManager.h"
#include "EditorRenderPipeline.h"
#include "Editors/EditorWindow.h"
#include "Editors/SceneEditor/SceneEditor.h"
#include "Importers/FBXImporter.h"
#include "Pulsar/Components/PointLightComponent.h"

#include "Shaders/EditorShaderCompileService.h"
#include "Utils/PrefabUtil.h"

#include <Pulsar/Rendering/ShaderInstanceCache.h>
#include <psc/ShaderCompiler.h>

#include <fstream>
#include <CoreLib.Serialization/JsonSerializer.h>
#include <CoreLib/File.h>
#include <Pulsar/Application.h>
#include <Pulsar/Components/BoxShape3DComponent.h>
#include <Pulsar/Components/DirectionalLightComponent.h>
#include <Pulsar/Components/SphereShape3DComponent.h>
#include <Pulsar/Components/StaticMeshRendererComponent.h>
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
        m_editors.push_back(std::make_unique<SceneEditor>());
    }
    const char* EditorAppInstance::AppType()
    {
        return "editor";
    }

    void EditorAppInstance::RequestQuit()
    {
        // SystemInterface::RequestQuitEvents();
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

    namespace ShapeMeshUtils
    {
        ObjectPtr<Node> CreateCube(
            RCPtr<Scene> scene,
            string name = "Node",
            RCPtr<Material> material = nullptr,
            bool dynamicBody = false,
            Vector3f position = Vector3f(),
            Vector3f rotation = Vector3f(),
            Vector3f scale = Vector3f(1.0f, 1.0f, 1.0f))
        {
            if (material == nullptr)
            {
                material = GetAssetManager()->LoadAsset<Material>("Engine/Materials/Lambert", true);
            }
            auto cube = GetAssetManager()->LoadAsset<StaticMesh>("Engine/Shapes/Cube", true);

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
            RCPtr<Scene> scene,
            string name = "Node",
            RCPtr<Material> material = nullptr,
            bool dynamicBody = false,
            Vector3f position = Vector3f(),
            Vector3f rotation = Vector3f(),
            Vector3f scale = Vector3f(1.0f, 1.0f, 1.0f))
        {
            auto sphere = GetAssetManager()->LoadAsset<StaticMesh>("Engine/Shapes/Sphere", true);
            if (material == nullptr)
            {
                material = GetAssetManager()->LoadAsset<Material>("Engine/Materials/Lambert", true);
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

    static void SetupDefaultResidentScene()
    {

        auto scene = World::Current()->GetResidentScene();
        // light
        {
            auto dlight = scene->NewNode("Directional Light");
            dlight->AddComponent<DirectionalLightComponent>();
            dlight->GetTransform()->TranslateRotateEuler({-3,3,-3}, {45,45,0});
        }

        return;
        // sky
        // if (0)
        {
            auto skySphere = scene->NewNode("Sky Sphere");
            skySphere->GetTransform()->SetScale({500,500,500});
            auto sphere = GetAssetManager()->LoadAsset<StaticMesh>(BuiltinAsset::Shapes_Sphere);
            auto renderer = skySphere->AddComponent<StaticMeshRendererComponent>();
            renderer->SetStaticMesh(sphere);
            renderer->SetMaterial(0, GetAssetManager()->LoadAsset<Material>("Engine/Materials/SkySphere"));
        }

        // default scene
        auto cube = GetAssetManager()->LoadAsset<StaticMesh>("Engine/Shapes/Cube", true);
        auto sphere = GetAssetManager()->LoadAsset<StaticMesh>("Engine/Shapes/Sphere", true);
        auto gridMat = GetAssetManager()->LoadAsset<Material>("Engine/Materials/WorldGrid", true);
        auto litMat = GetAssetManager()->LoadAsset<Material>("Engine/Materials/Lit", true);

        {
            ShapeMeshUtils::CreateCube(scene, "floor", gridMat, false, {0, -0.25f, 0},{}, { 10.f, 0.5f, 10.f});
            ShapeMeshUtils::CreateCube(scene, "cube", litMat,true, {-2.f,0.5f,2.f},{0.0f, -35.0f,0.0f});
            ShapeMeshUtils::CreateCube(scene, "cube", litMat,true, {0.f,0.5f,2.f},{0.0f, -10.0f,0.0f});
            ShapeMeshUtils::CreateCube(scene, "cube", litMat, true, {-0.45f, 1.5f, 2.f}, {0.0f, -40.0f, 0.0f});
            ShapeMeshUtils::CreateCube(scene, "cube", litMat, true, {-1.65f, 1.25f, 2.f}, {0.0f, -10.0f, 0.0f}, {0.5f, 0.5f, 0.5f});
            ShapeMeshUtils::CreateSphere(scene, "sphere", litMat, true, {-0.3f, 5.f, 1.3f});

        }
        {
            auto gamma = GetAssetManager()->LoadAsset<Material>("Engine/Materials/GammaCorrection", true);
            World::Current()->GetCurrentCamera()->AddPostProcess(gamma);
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

    static void PreCompileShaders()
    {
        // TODO: 旧的预编译流程已废弃，shader 编译现在通过
        // ShaderInstanceCache + EditorShaderCompileService 按需触发
    }

    static void PreImportBuiltinModel()
    {
        {
            FBXImporter importer;
            FBXImporterSettings settings;

            auto cube = AssetDatabase::GetPackagePhysicsPath("Engine") / "SrcModel/Cube.fbx";
            settings.ImportFiles->push_back(jxcorlib::PathToU8Str(cube));
            settings.ImportingTargetFolder = "Engine/Shapes";

            auto imported = importer.Import(&settings);
            for (auto item : imported)
            {
                AssetDatabase::Save(item);
            }
        }
        {
            FBXImporter importer;
            FBXImporterSettings settings;

            auto sphere = AssetDatabase::GetPackagePhysicsPath("Engine") / "SrcModel/Sphere.fbx";
            settings.ImportFiles->push_back(jxcorlib::PathToU8Str(sphere));
            settings.ImportingTargetFolder = "Engine/Shapes";

            auto imported = importer.Import(&settings);
            for (auto item : imported)
            {
                AssetDatabase::Save(item);
            }
        }

        {
            FBXImporter importer;
            FBXImporterSettings settings;

            auto sphere = AssetDatabase::GetPackagePhysicsPath("Engine") / "SrcModel/Plane.fbx";
            settings.ImportFiles->push_back(jxcorlib::PathToU8Str(sphere));
            settings.ImportingTargetFolder = "Engine/Shapes";

            auto imported = importer.Import(&settings);
            for (auto item : imported)
            {
                AssetDatabase::Save(item);
            }
        }
    }

    void EditorAppInstance::OnInitialized()
    {
        m_assetManager = new EditorAssetManager;
        OnCreateEditors();

        // search package path
        AssetDatabase::Initialize();
        AssetDatabase::AddProgramPackageSearchPath(_SearchUpFolder("Packages"));

        // add package
        AssetDatabase::AddPackage("Engine");
        AssetDatabase::AddPackage("Editor");

        // 注册 Shader 编译服务
        static EditorShaderCompileService s_shaderCompileService;
        pulsar::ShaderCompileServiceLocator::Register(&s_shaderCompileService);

        // 同步编译 Pending/Error shader 并初始化 ShaderInstanceCache
        {
            auto pendingProgram = std::make_shared<pulsar::ShaderProgramResource>();
            auto errorProgram = std::make_shared<pulsar::ShaderProgramResource>();

            auto CompileBuiltinShader = [&](const std::string& assetPath, std::shared_ptr<pulsar::ShaderProgramResource>& outProgram)
            {
                auto hlslAssetPath = assetPath + ".hlsl";
                auto hlslPhysicsPath = AssetDatabase::AssetPathToPhysicsPath(hlslAssetPath);
                auto includeDir = hlslPhysicsPath.parent_path();

                std::ifstream hlslFile(hlslPhysicsPath);
                if (!hlslFile.is_open())
                {
                    Logger::Log("Failed to open builtin shader: " + hlslPhysicsPath.string(), pulsar::LogLevel::Error);
                    return;
                }
                auto hlslSource = std::string(
                    (std::istreambuf_iterator<char>(hlslFile.rdbuf())),
                    std::istreambuf_iterator<char>());

                auto pscApi = psc::ApiPlatformType::Vulkan;
                auto pscCompiler = psc::CreateShaderCompiler(pscApi);
                auto gfxApp = pulsar::Application::GetGfxApp();

                // VS
                {
                    psc::CompileInfo info{};
                    info.code = hlslSource.c_str();
                    info.platform = pscApi;
                    info.Stage = psc::FilePartialType::Vert;
                    info.EntryName = "VSMain";
                    info.IncludePaths = { includeDir };
                    auto spirv = pscCompiler->CompileStage(info);
                    auto vsProgram = gfxApp->CreateGpuProgram(
                        gfx::GFXGpuProgramStageFlags::Vertex,
                        reinterpret_cast<const uint8_t*>(spirv.data()), spirv.size());
                    vsProgram->SetEntryName("VSMain");
                    outProgram->m_gpuPrograms.push_back(vsProgram);
                }
                // PS
                {
                    psc::CompileInfo info{};
                    info.code = hlslSource.c_str();
                    info.platform = pscApi;
                    info.Stage = psc::FilePartialType::Pixel;
                    info.EntryName = "PSMain";
                    info.IncludePaths = { includeDir };
                    auto spirv = pscCompiler->CompileStage(info);
                    auto psProgram = gfxApp->CreateGpuProgram(
                        gfx::GFXGpuProgramStageFlags::Fragment,
                        reinterpret_cast<const uint8_t*>(spirv.data()), spirv.size());
                    psProgram->SetEntryName("PSMain");
                    outProgram->m_gpuPrograms.push_back(psProgram);
                }
            };

            CompileBuiltinShader("Engine/Shaders/Pending", pendingProgram);
            CompileBuiltinShader("Engine/Shaders/Error", errorProgram);

            pulsar::ShaderInstanceCache::Instance().Initialize(pendingProgram, errorProgram);
            Logger::Log("ShaderInstanceCache initialized with Pending/Error programs");
        }

        Logger::Log("precompile shaders...");
        // recompile obsolete shaders
        PreCompileShaders();
        // PreImportBuiltinModel();

        // world
        Logger::Log("initialize world");
        auto edWorld = World::Reset<EditorWorld>("MainWorld");
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
            const auto defaultLayoutPath = AssetDatabase::PackagePathToPhysicsPath("Editor/Layout/Default.ini");
            m_gui->SetLayoutInfo(FileUtil::ReadAllText(defaultLayoutPath));
        }

        _RegisterIcon(cltypeof<FolderAsset>(), "Editor/Icons/folder.png");
        _RegisterIcon(cltypeof<Shader>(), "Editor/Icons/shader.png");
        _RegisterIcon(cltypeof<Material>(), "Editor/Icons/material.png");
        _RegisterIcon(cltypeof<StaticMesh>(), "Editor/Icons/staticmesh.png");
        _RegisterIcon(cltypeof<Texture2D>(), "Editor/Icons/texture.png");
        _RegisterIcon(cltypeof<Prefab>(), "Editor/Icons/prefab.png");
        _RegisterIcon(cltypeof<ObjectBase>(), "Editor/Icons/object.png");
        _RegisterIcon("WorkspaceWindow.Dirty", "Editor/Icons/Star.png");


        Logger::Log("initialize subsystems");

        // init window ui
        Logger::Log("initialize editor window manager");
        EditorWindowManager::Initialize();

        for (auto& editor : m_editors)
        {
            editor->Initialize();
        }
        m_editors[0]->CreateEditorWindow()->Open();

        Workspace::OpenWorkspace(_SearchUpFolder("Project") / "Project.peproj");
        SetupDefaultResidentScene();

        uinput::InputManager::GetInstance()->Initialize();
    }

    void EditorAppInstance::OnTerminate()
    {
        uinput::InputManager::GetInstance()->Terminate();

        for (auto& editor : m_editors)
        {
            editor->Terminate();
        }
        m_editors.clear();

        PrefabUtil::ClosePrefabMode();

        // 在 World 和 GFX Device 销毁前清理 ShaderInstanceCache，
        // 否则 GpuProgram 析构时 VkDevice 已经无效
        pulsar::ShaderInstanceCache::Instance().Clear();

        World::Reset(nullptr);

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

        delete m_assetManager;
        m_assetManager = nullptr;
        AssetDatabase::Terminate();
        RuntimeObjectManager::Terminate();
        EditorLogRecorder::Terminate();
    }

    void EditorAppInstance::OnBeginRender(float dt)
    {
        // 刷新异步 shader 编译回调（主线程）
        if (auto* compileService = dynamic_cast<EditorShaderCompileService*>(pulsar::ShaderCompileServiceLocator::Get()))
        {
            compileService->FlushCallbacks();
        }

        m_gui->NewFrame();

        uinput::InputManager::GetInstance()->ProcessEvents();

        EditorWorld::GetPreviewWorld()->Tick(dt);
        //World::Current()->Tick(dt);

        pulsared::EditorWindowManager::Draw(dt);
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

    void EditorAppInstance::OnEndRender(float dt)
    {
        RuntimeObjectManager::TickGCollect();
    }

    bool EditorAppInstance::IsQuit()
    {
        // return SystemInterface::GetIsQuit();
        return false;
    }


    Vector2f EditorAppInstance::GetAppSize()
    {
        int32_t w, h;
        Application::GetGfxApp()->GetViewport()->GetSize(&w, &h);
        return {(float)w, (float)h};
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
    void EditorAppInstance::ShowModalDialog(SPtr<ModalDialog> dialog)
    {
        m_modalDialog = std::move(dialog);
    }


}
