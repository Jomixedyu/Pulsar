#include "EditorAppInstance.h"
#include "EditorAssetManager.h"
#include "EditorRenderPipeline.h"
#include "Editors/SceneEditor/SceneEditor.h"
#include "Pulsar/Components/DirectionalLightComponent.h"
#include "Pulsar/Components/StaticMeshRendererComponent.h"
#include "Pulsar/Prefab.h"
#include "Shaders/EditorShader.h"
#include "Utils/PrefabUtil.h"
#include <CoreLib.Serialization/JsonSerializer.h>
#include <CoreLib/File.h>
#include <Pulsar/Application.h>
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

    static void _Test()
    {
        auto vertexColorMat = GetAssetManager()->LoadAsset<Material>("Engine/Materials/VertexColor", true);
        {
            auto dlight = World::Current()->GetResidentScene()->NewNode("Directional Light");
            dlight->AddComponent<DirectionalLightComponent>();

            dlight->GetTransform()->TranslateRotateEuler({-3,3,-3}, {45,45,0});
        }

        array_list<StaticMeshSection> sections;
        auto& section = sections.emplace_back();
        StaticMeshVertex vert0{};
        vert0.Position = {0,0.5,0};
        StaticMeshVertex vert1{};
        vert1.Position = {1,0,0};
        StaticMeshVertex vert2{};
        vert2.Position = {0,0,3};

        section.Vertex.push_back(vert0);
        section.Vertex.push_back(vert1);
        section.Vertex.push_back(vert2);
        section.Indices.push_back(0);
        section.Indices.push_back(1);
        section.Indices.push_back(2);

        auto sm = StaticMesh::StaticCreate("aa", std::move(sections), {});
        World::Current()->GetResidentScene()->NewNode("sm")->AddComponent<StaticMeshRendererComponent>()->SetStaticMesh(sm);
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
        array_list<string_view> PreCompileShaderPaths = {
            "Engine/Shaders/Missing"
        };
        if (PreCompileShaderPaths.empty())
        {
            return;
        }
        for (auto& element : AssetDatabase::FindAssets(cltypeof<Shader>()))
        {
            if (std::ranges::contains(PreCompileShaderPaths, element))
            {
                auto asset = cref_cast<Shader>(AssetDatabase::LoadAssetAtPath(element));
                assert(asset);
                ShaderCompiler::CompileShader(asset.GetPtr(), {gfx::GFXApi::Vulkan}, {}, {});
            }
        }
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

        // recompile obsolete shaders
        PreCompileShaders();

        // world
        Logger::Log("initialize world");
        auto edWorld = World::Reset<EditorWorld>("MainWorld");
        edWorld->GetCurrentCamera()->GetTransform()->GetParent()->SetEuler({45.f,-45,0});

        m_world = edWorld;
        // {
        //     auto skySphere = edWorld->GetResidentScene()->NewNode("Sky Sphere");
        //     skySphere->GetTransform()->SetScale({500,500,500});
        //     auto sphere = GetAssetManager()->LoadAsset<StaticMesh>(BuiltinAsset::Shapes_Sphere);
        //     auto renderer = skySphere->AddComponent<StaticMeshRendererComponent>();
        //     renderer->SetStaticMesh(sphere);
        //     renderer->SetMaterial(0, GetAssetManager()->LoadAsset<Material>("Engine/Materials/SkySphere"));
        // }

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

        // InitBasicMenu();



        Logger::Log("initialize subsystems");

        // init window ui
        Logger::Log("initialize editor window manager");
        EditorWindowManager::Initialize();

        auto se = new SceneEditor;
        se->Initialize();
        se->CreateEditorWindow()->Open();

        // _InitWindowMenu();

        Workspace::OpenWorkspace(_SearchUpFolder("Project") / "Project.peproj");
        // _Test();

        uinput::InputManager::GetInstance()->Initialize();
    }

    void EditorAppInstance::OnTerminate()
    {
        uinput::InputManager::GetInstance()->Terminate();

        PrefabUtil::ClosePrefabMode();
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

    rendering::Pipeline* EditorAppInstance::GetPipeline()
    {
        return nullptr;
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
