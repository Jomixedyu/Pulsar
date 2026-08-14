#include "World.h"
#include "Application.h"
#include "Input.h"

#include <Pulsar/Logger.h>
#include <Pulsar/Scene.h>
#include <Pulsar/Assets/NodeCollection.h>

#include "Components/RenderComponent.h"
#include "Components/SceneCaptureComponent.h"
#include "Physics2D/PhysicsWorld2D.h"
#include "Physics3D/PhysicsWorld3D.h"
#include "Rendering/LightingData.h"
#include "Rendering/RenderThread.h"
#include "Rendering/SceneView.h"
#include "Subsystems/Subsystem.h"
#include "Subsystems/WorldSubsystem.h"

namespace pulsar
{
    static hash_set<World*> gWorlds;
    const hash_set<World*>& World::GetAllWorlds()
    {
        return gWorlds;
    }
    World::World(string_view name)
        : m_name(name), m_gizmosManager(this), m_inputContext(new InputContext())
    {
        gWorlds.insert(this);
    }

    World::~World()
    {
        delete m_inputContext;
        gWorlds.erase(this);
        TeardownRenderScene();
    }

    void World::OnDuplicated(World* target)
    {
        target->m_name = m_name + "_copy";
        for (size_t i = 1; i < m_scenes.size(); ++i)
        {
            target->LoadScene(InstantiateAsset(m_scenes[i]));
        }
    }

    void World::BeginPlay()
    {
        if (m_isPlaying)
            return;
        m_isPlaying = true;
        for (auto& subsystem : m_subsystems)
        {
            subsystem->OnBeginPlay();
        }
        for (auto& scene : m_scenes)
        {
            if (scene)
            {
                scene->BeginPlay();
            }
        }

        BeginSimulate();
    }

    void World::EndPlay()
    {
        if (!m_isPlaying)
            return;
        m_isPlaying = false;
        m_isPaused = false;
        for (auto& scene : m_scenes)
        {
            if (scene)
            {
                scene->EndPlay();
            }
        }
        for (auto& subsystem : m_subsystems)
        {
            subsystem->OnEndPlay();
        }
        EndSimulate();
    }

    void World::BeginSimulate()
    {
        if (m_isSimulating)
            return;
        m_isSimulating = true;
        for (auto& simulate : m_simulateManager.GetSimulates())
        {
            simulate->BeginSimulate();
        }
        m_physicsWorld2D->BeginSimulate();
        m_physicsWorld3D->BeginSimulate();
    }
    void World::EndSimulate()
    {
        if (!m_isSimulating)
            return;
        m_isSimulating = false;
        for (auto& simulate : m_simulateManager.GetSimulates())
        {
            simulate->EndSimulate();
        }
        m_physicsWorld2D->EndSimulate();
        m_physicsWorld3D->EndSimulate();
    }

    void World::CameraFocusNode(Node* node)
    {

    }

    void World::BeginInputFrame()
    {
        if (m_isPlaying && m_inputContext)
            m_inputContext->BeginFrame();
    }

    void World::ProcessInputEvent(const uinput::InputEvent& e)
    {
        if (m_isPlaying && m_inputContext)
            m_inputContext->ProcessEvent(e);
    }

    void World::Tick(float dt)
    {
        if (!m_isPaused)
        {
            m_ticker += dt;
        }
        m_totalTime += dt;

        m_gizmosManager.Draw();

        if (m_isPlaying && !m_isPaused)
        {
            for (auto& scene : m_scenes)
            {
                if (scene)
                {
                    scene->Tick(m_ticker);
                }
            }
        }
        if ((m_isPlaying && !m_isPaused) || m_isSimulating)
        {
            m_physicsWorld2D->Tick(dt);
            m_physicsWorld3D->StepSimulate(dt);
        }
        if (m_isSimulating)
        {
            m_simulateManager.SimulateTick(dt);
        }

        // Extraction phase: runs unconditionally every frame (play or not), after all
        // node ticks/tools/gizmos have mutated state. Drains the proxy-update list.
        SyncRenderProxies();
    }

    bool World::IsSelectedNode(const ObjectPtr<Node>& node) const
    {
        return std::ranges::any_of(m_selection.GetSelection(), [&](auto& i){ return i == node; });
    }
    ObjectPtr<Node> World::FindNodeByName(string_view name, bool includeInactive) const
    {
        for (auto& scene : m_scenes)
        {
            if (auto node = scene->FindNodeByName(name))
            {
                return node;
            }
        }
        return {};
    }
    WorldSubsystem* World::GetSubsystem(Type* type) const
    {
        for (auto& subsystem : m_subsystems)
        {
            if (subsystem->GetType() == type)
            {
                return subsystem.get();
            }
        }
        return {};
    }


    ObjectPtr<CameraComponent> World::GetCurrentCamera()
    {
        return GetCameraManager().GetMainCamera();
    }
    void World::SetFocusScene(RCPtr<NodeCollection> scene)
    {
        m_focusScene = scene;
    }

    void World::ChangeScene(RCPtr<NodeCollection> scene, bool clearResidentScene)
    {
        if (clearResidentScene)
        {
            UnloadAllScene(true);
            InitializeResidentScene();
        }
        else
        {
            UnloadAllScene(false);
        }
        LoadScene(scene);
    }

    void World::LoadScene(RCPtr<NodeCollection> scene)
    {
        m_scenes.push_back(scene);
        this->OnSceneLoading(scene);
        scene->BeginScene(this);
    }

    void World::UnloadScene(RCPtr<NodeCollection> scene)
    {
        const auto it = std::ranges::find(m_scenes, scene);
        if (it == m_scenes.end())
        {
            return;
        }
        if (it == m_scenes.begin())
        {
            OnUnloadingResidentScene(scene);
        }
        else
        {
            if (m_focusScene == scene)
            {
                m_focusScene = GetResidentScene();
            }
        }
        OnSceneUnloading(scene);
        scene->EndScene();
        m_scenes.erase(it);
    }

    void World::InitializeResidentScene()
    {
        auto scene = Scene::StaticCreate("ResidentScene");
        scene->SetObjectFlags(scene->GetObjectFlags() | OF_Transient & ~OF_Instantiable);

        LoadScene(scene);
        OnLoadingResidentScene(scene);
    }

    void World::UnloadAllScene(bool unloadResidentScene)
    {
        auto scenes = m_scenes;
        auto startIndex = unloadResidentScene ? 0 : 1;
        for (int i = (int)scenes.size() - 1; i >= 0; i--)
        {
            UnloadScene(scenes[i]);
        }
    }

    void World::OnLoadingResidentScene(RCPtr<NodeCollection> scene)
    {
    }
    void World::OnUnloadingResidentScene(RCPtr<NodeCollection> scene)
    {
    }
    void World::AddRenderObject(const rendering::RenderObject_sp& ro)
    {
        if (!m_renderScene || !ro)
            return;
        Application::GetRenderThread()->EnqueueUpdate_AnyThread(
            [scene = m_renderScene.get(), p = ro](gfx::GFXResourceManager*) mutable
            {
                scene->AddProxy(std::move(p));
            });
    }
    void World::RemoveRenderObject(const rendering::RenderObject_sp& ro)
    {
        if (!m_renderScene || !ro)
            return;
        Application::GetRenderThread()->EnqueueUpdate_AnyThread(
            [scene = m_renderScene.get(), p = ro](gfx::GFXResourceManager*) mutable
            {
                scene->RemoveProxy(p);
            });
    }
    void World::RegisterProxy(RenderComponent* comp)
    {
        if (!m_renderScene || !comp)
            return;
        // Build the proxy from the component and store it back on the component.
        auto proxy = comp->CreateRenderProxy();
        comp->m_proxy = proxy;
        if (!proxy)
            return;

        Application::GetRenderThread()->EnqueueUpdate_AnyThread(
            [scene = m_renderScene.get(), p = std::move(proxy)](gfx::GFXResourceManager*) mutable
            {
                scene->AddProxy(std::move(p));
            });
    }
    void World::UnregisterProxy(RenderComponent* comp)
    {
        if (!comp || !comp->m_proxy)
            return;
        auto proxy = std::move(comp->m_proxy);
        comp->m_proxy.reset();
        if (!m_renderScene)
            return;

        Application::GetRenderThread()->EnqueueUpdate_AnyThread(
            [scene = m_renderScene.get(), p = std::move(proxy)](gfx::GFXResourceManager*) mutable
            {
                scene->RemoveProxy(p);
            });
    }
    void World::UpdateSceneView(const SPtr<SceneView>& view, SceneViewData data)
    {
        if (!m_renderScene || !view)
            return;
        // Submission API: game thread only. The view proxy is kept alive by the
        // render scene; the captured SPtr keeps it alive until the update runs.
        Application::GetRenderThread()->EnqueueUpdate_AnyThread(
            [view, data = std::move(data)](gfx::GFXResourceManager*) mutable
            {
                view->SetData(std::move(data));
            });
    }
    void World::TeardownRenderScene()
    {
        if (!m_renderScene)
            return;

        // Having m_renderScene means the World is running with a render thread.
        // Headless worlds do not create RenderScene and return above.
        // Use the destroy queue so the scene is destroyed after previously queued proxy updates.
        Application::GetRenderThread()->EnqueueDestroy_AnyThread(
            [scene = std::move(m_renderScene)](gfx::GFXResourceManager*) mutable
            {
                scene->Destroy();
            });
    }

    void World::MarkProxyDirty(RenderComponent* comp)
    {
        // Dedup is guaranteed by RenderComponent::m_renderStateDirty (set before this
        // is called), so a plain push_back never produces duplicates.
        m_pendingProxyUpdates.push_back(comp);
    }

    void World::UnmarkProxyDirty(RenderComponent* comp)
    {
        auto it = std::ranges::find(m_pendingProxyUpdates, comp);
        if (it != m_pendingProxyUpdates.end())
        {
            *it = m_pendingProxyUpdates.back();
            m_pendingProxyUpdates.pop_back();
        }
    }

    void World::SyncRenderProxies()
    {
        // Push the world-level time snapshot to the render scene every frame (it changes
        // each tick), so renderers read it from the render thread instead of live World.
        if (m_renderScene)
        {
            Application::GetRenderThread()->EnqueueUpdate_AnyThread(
                [scene = m_renderScene.get(), totalTime = m_totalTime, deltaTime = m_ticker.deltatime](gfx::GFXResourceManager*) mutable
                {
                    scene->SetTime(totalTime, deltaTime);
                });
        }

        if (m_pendingProxyUpdates.empty())
            return;
        // Move out so a SyncRenderProxy() that re-marks dirty re-registers into a fresh
        // list (processed next frame) rather than mutating the one we iterate.
        auto pending = std::move(m_pendingProxyUpdates);
        m_pendingProxyUpdates.clear();
        for (auto* comp : pending)
        {
            comp->m_renderStateDirty = false;
            comp->SyncRenderProxy();
        }
    }

    void World::OnWorldBegin()
    {
        if (!m_renderScene)
        {
            m_renderScene = std::make_unique<RenderScene>();
        }
        if (m_scenes.empty())
        {
            InitializeResidentScene();
        }
        m_focusScene = GetResidentScene();

        m_physicsWorld2D = new PhysicsWorld2D;
        m_physicsWorld3D = new PhysicsWorld3D;

        for (auto& item : SubsystemManager::GetAllSubsystems())
        {
            if (item->IsSubclassOf(cltypeof<WorldSubsystem>()))
            {
                auto subsystem = sptr_cast<WorldSubsystem>(item->CreateSharedInstance({}));
                assert(subsystem);

                subsystem->m_world = this;
                subsystem->OnInitializing();
                m_subsystems.push_back(subsystem);
            }
        }
        for (auto& subsystem : m_subsystems)
        {
            subsystem->OnInitialized();
        }
    }

    void World::OnWorldEnd()
    {
        for (auto& subsystem : m_subsystems)
        {
            subsystem->OnTerminate();
        }
        m_subsystems.clear();

        UnloadAllScene();
        for (const auto& i : m_deferredDestroyedQueue)
        {
            DestroyObject(i);
        }
        m_deferredDestroyedQueue.clear();

        delete m_physicsWorld2D;
        m_physicsWorld2D = nullptr;

        delete m_physicsWorld3D;
        m_physicsWorld3D = nullptr;

        TeardownRenderScene();

        // Synchronous unload: drain proxy removals / RenderScene destroy, then wait for GPU idle.
        if (auto* rt = Application::GetRenderThread())
            rt->WaitForIdle_AnyThread();
    }

    void World::OnSceneLoading(RCPtr<NodeCollection> scene)
    {
    }

    void World::OnSceneUnloading(RCPtr<NodeCollection> scene)
    {
    }

} // namespace pulsar
