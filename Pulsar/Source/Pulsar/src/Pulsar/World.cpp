#include "World.h"
#include "Application.h"

#include <Pulsar/Logger.h>
#include <Pulsar/Scene.h>
#include <Pulsar/Assets/NodeCollection.h>

#include "Physics2D/PhysicsWorld2D.h"
#include "Physics3D/PhysicsWorld3D.h"
#include "Rendering/LightingData.h"
#include "Subsystems/Subsystem.h"
#include "Subsystems/WorldSubsystem.h"

namespace pulsar
{
    static std::unique_ptr<World> _world_inst = nullptr;
    World* World::Current()
    {
        return _world_inst.get();
    }

    World* World::Reset(std::unique_ptr<World>&& world)
    {
        if (_world_inst)
        {
            _world_inst->OnWorldEnd();
        }
        _world_inst = std::move(world);
        if (_world_inst)
        {
            _world_inst->OnWorldBegin();
        }
        return _world_inst.get();
    }

    static hash_set<World*> gWorlds;
    const hash_set<World*>& World::GetAllWorlds()
    {
        return gWorlds;
    }
    World::World(string_view name)
        : m_name(name), m_gizmosManager(this)
    {
        gWorlds.insert(this);
    }

    World::~World()
    {
        gWorlds.erase(this);
    }

    void World::OnDuplicated(World* target)
    {
        target->m_name = m_name + "_copy";
        for (auto& scene : m_scenes)
        {
            target->m_scenes.push_back(InstantiateAsset(scene));
        }
    }

    void World::BeginPlay()
    {
        if (m_isPlaying)
            return;
        m_isPlaying = true;
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
        for (auto& scene : m_scenes)
        {
            if (scene)
            {
                scene->EndPlay();
            }
        }
        EndSimulate();
    }

    void World::BeginSimulate()
    {
        for (auto& simulate : m_simulateManager.GetSimulates())
        {
            simulate->BeginSimulate();
        }
        m_physicsWorld2D->BeginSimulate();
        m_physicsWorld3D->BeginSimulate();
    }
    void World::EndSimulate()
    {
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

    void World::Tick(float dt)
    {
        m_ticker += dt;
        m_totalTime += dt;

        m_lightManager->Update();

        m_gizmosManager.Draw();

        if (m_isPlaying)
        {
            for (auto& scene : m_scenes)
            {
                if (scene)
                {
                    scene->Tick(m_ticker);
                }
            }
            m_physicsWorld2D->Tick(dt);
            m_physicsWorld3D->StepSimulate(dt);
        }
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
    void World::AddRenderObject(const rendering::RenderObject_sp& renderObject)
    {
        renderObject->OnCreateResource();
        m_renderObjects.insert(renderObject);
    }
    void World::RemoveRenderObject(rendering::RenderObject_rsp renderObject)
    {
        const auto it = m_renderObjects.find(renderObject);
        if (it != m_renderObjects.end())
        {
            (*it)->OnDestroyResource();
            m_renderObjects.erase(it);
        }
    }

    void World::OnWorldBegin()
    {
        #ifdef WITH_EDITOR
        assert(GetWorldTypeName() != StaticWorldTypeName());
        #endif

        InitializeResidentScene();
        m_focusScene = GetResidentScene();

        m_physicsWorld2D = new PhysicsWorld2D;
        m_physicsWorld3D = new PhysicsWorld3D;

        m_lightManager = new LightManager;

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

        delete m_lightManager;
        m_lightManager = nullptr;
    }

    void World::OnSceneLoading(RCPtr<NodeCollection> scene)
    {
    }

    void World::OnSceneUnloading(RCPtr<NodeCollection> scene)
    {
    }

} // namespace pulsar
