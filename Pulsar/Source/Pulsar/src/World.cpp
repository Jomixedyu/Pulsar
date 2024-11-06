#include "World.h"
#include "Application.h"

#include <Pulsar/Logger.h>
#include <Pulsar/Scene.h>

#include "Physics2D/PhysicsWorld2D.h"
#include "Subsystems/Subsystem.h"
#include "Subsystems/WorldSubsystem.h"

namespace pulsar
{
    struct WorldRenderBufferData
    {
        Vector4f WorldSpaceLightVector;
        Vector4f WorldSpaceLightColor; // w is intensity
        float TotalTime;
        float DeltaTime;
        Vector2f _Padding0;
        Vector4f SkyLightColor; // w is intensity
    };

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
            target->m_scenes.push_back(scene->InstantiateAsset());
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
        physicsWorld2D->BeginSimulate();
    }
    void World::EndSimulate()
    {
        for (auto& simulate : m_simulateManager.GetSimulates())
        {
            simulate->EndSimulate();
        }
        physicsWorld2D->EndSimulate();
    }

    void World::Tick(float dt)
    {
        m_ticker += dt;
        m_totalTime += dt;

        UpdateWorldCBuffer();

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
            physicsWorld2D->Tick(dt);
        }
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
    int64_t World::AllocElementId(guid_t obj)
    {
        static int64_t i = 0;
        ++i;
        m_elementIdMap[i] = obj;
        return i;
    }
    void World::FreeElementId(int64_t id)
    {
        m_elementIdMap.erase(id);
    }
    guid_t World::FindElementId(int64_t id)
    {
        return m_elementIdMap[id];
    }

    ObjectPtr<CameraComponent> World::GetCurrentCamera()
    {
        return GetCameraManager().GetMainCamera();
    }
    void World::SetFocusScene(RCPtr<Scene> scene)
    {
        m_focusScene = scene;
    }

    void World::ChangeScene(RCPtr<Scene> scene, bool clearResidentScene)
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

    void World::LoadScene(RCPtr<Scene> scene)
    {
        m_scenes.push_back(scene);
        this->OnSceneLoading(scene);
        scene->BeginScene(this);
    }

    void World::UnloadScene(RCPtr<Scene> scene)
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
            if (m_focusScene.Handle == scene.Handle)
            {
                m_focusScene = GetResidentScene();
            }
        }
        OnSceneUnloading(scene);
        scene->EndScene();
        m_scenes.erase(it);
        DestroyObject(scene, true);
    }

    void World::InitializeResidentScene()
    {
        auto scene = Scene::StaticCreate("ResidentScene");
        scene->SetObjectFlags(scene->GetObjectFlags() | OF_Instance & ~OF_Persistent & ~OF_Instantiable);

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

    void World::OnLoadingResidentScene(RCPtr<Scene> scene)
    {
    }
    void World::OnUnloadingResidentScene(RCPtr<Scene> scene)
    {
    }
    void World::UpdateWorldCBuffer()
    {
        WorldRenderBufferData buffer{};
        buffer.DeltaTime = m_ticker.deltatime;
        buffer.TotalTime = m_totalTime;

        auto& sceneEnv = m_focusScene->GetRuntimeEnvironment();

        if (const auto dirLight = sceneEnv.GetDirectionalLight())
        {
            buffer.WorldSpaceLightVector = dirLight->Vector;
            auto c = dirLight->Color;
            buffer.WorldSpaceLightColor = {c.r, c.g, c.b, c.a};
            buffer.WorldSpaceLightColor.w = dirLight->Intensity;
        }

        {
            auto skyLight = sceneEnv.GetSkyLight();
            buffer.SkyLightColor = {skyLight.Color.r, skyLight.Color.g, skyLight.Color.b, 1};
            buffer.SkyLightColor.w = skyLight.Intensity;
        }

        m_worldDescriptorBuffer->Fill(&buffer);
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

        gfx::GFXDescriptorSetLayoutInfo info{
            gfx::GFXDescriptorType::ConstantBuffer,
            gfx::GFXShaderStageFlags::VertexFragment,
            0, kRenderingDescriptorSpace_World};

        m_worldDescriptorLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(&info, 1);
        m_worldDescriptorBuffer = Application::GetGfxApp()->CreateBuffer(
            gfx::GFXBufferUsage::ConstantBuffer,
            sizeof(WorldRenderBufferData));

        m_worldDescriptors = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_worldDescriptorLayout);
        m_worldDescriptors->AddDescriptor("World", 0)->SetConstantBuffer(m_worldDescriptorBuffer.get());
        m_worldDescriptors->Submit();

        physicsWorld2D = new PhysicsWorld2D();

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

        m_worldDescriptorLayout.reset();
        m_worldDescriptorBuffer.reset();
        m_worldDescriptors.reset();

        delete physicsWorld2D;
        physicsWorld2D = nullptr;
    }

    void World::OnSceneLoading(RCPtr<Scene> scene)
    {
    }

    void World::OnSceneUnloading(RCPtr<Scene> scene)
    {
    }

} // namespace pulsar
