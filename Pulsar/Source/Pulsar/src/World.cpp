#include "World.h"
#include "Application.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Scene.h>

namespace pulsar
{
    struct CBuffer_World
    {
        Vector4f WorldSpaceLightPos;
        Vector4f WorldSpaceLightColor;
        float TotalTime;
        float DeltaTime;
        Vector2f _Padding0;
        Vector4f _Padding1;
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
    World::World()
    {
    }

    World::~World()
    {
    }

    void World::Tick(float dt)
    {
        m_ticker += dt;
        m_totalTime += dt;

        UpdateWorldCBuffer();

        for (auto& scene : m_scenes)
        {
            if (IsValid(scene))
            {
                for (auto& node : *scene->GetNodes())
                {
                    if (IsValid(node) && node->GetIsActive())
                    {
                        node->OnTick(m_ticker);
                    }
                }
            }
        }
    }
    CameraComponent_ref World::GetPreviewCamera()
    {
        return GetCameraManager().GetMainCamera();
    }
    void World::SetFocusScene(ObjectPtr<Scene> scene)
    {
        m_focusScene = scene;
    }

    void World::ChangeScene(ObjectPtr<Scene> scene, bool clearPresistentScene)
    {
        if (clearPresistentScene)
        {
            UnloadAllScene(true);
            InitializePersistentScene();
        }
        else
        {
            UnloadAllScene(false);
        }
        LoadScene(scene);
    }

    void World::LoadScene(ObjectPtr<Scene> scene)
    {
        m_scenes.push_back(scene);
        this->OnSceneLoading(scene);
        scene->BeginScene(this);
    }
    void World::UnloadScene(ObjectPtr<Scene> scene)
    {
        const auto it = std::ranges::find(m_scenes, scene);
        if (it == m_scenes.end())
        {
            return;
        }
        if (it == m_scenes.begin())
        {
            OnUnloadingPersistentScene(scene);
        }
        else
        {
            if (m_focusScene == scene)
            {
                m_focusScene = GetPersistentScene();
            }
        }
        OnSceneUnloading(scene);
        scene->EndScene();
        m_scenes.erase(it);
        DestroyObject(scene, true);
    }

    void World::InitializePersistentScene()
    {
        auto scene = Scene::StaticCreate("PresistentScene");
        scene->SetObjectFlags(scene->GetObjectFlags() | OF_Instance);

        LoadScene(scene);
        OnLoadingPersistentScene(scene);
    }
    void World::UnloadAllScene(bool unloadPresistentScene)
    {
        auto scenes = m_scenes;
        auto startIndex = unloadPresistentScene ? 0 : 1;
        for (int i = (int)scenes.size() - 1; i >= 0; i--)
        {
            UnloadScene(scenes[i]);
        }
    }

    void World::OnLoadingPersistentScene(ObjectPtr<Scene> scene)
    {
    }
    void World::OnUnloadingPersistentScene(ObjectPtr<Scene> scene)
    {
    }
    void World::UpdateWorldCBuffer()
    {
        CBuffer_World data;
        data.DeltaTime = m_ticker.deltatime;
        data.TotalTime = m_totalTime;

        m_worldDescriptorBuffer->Fill(&data);
    }

    void World::AddRenderObject(const rendering::RenderObject_sp renderObject)
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
        InitializePersistentScene();
        m_focusScene = GetPersistentScene();

        gfx::GFXDescriptorSetLayoutInfo info{
            gfx::GFXDescriptorType::ConstantBuffer,
            gfx::GFXShaderStageFlags::VertexFragment,
            0, kRenderingDescriptorSpace_World};

        m_worldDescriptorLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(&info, 1);
        m_worldDescriptorBuffer = Application::GetGfxApp()->CreateBuffer(
            gfx::GFXBufferUsage::ConstantBuffer,
            sizeof(CBuffer_World));

        m_worldDescriptors = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_worldDescriptorLayout);
        m_worldDescriptors->AddDescriptor("World", 0)->SetConstantBuffer(m_worldDescriptorBuffer.get());
        m_worldDescriptors->Submit();
    }

    void World::OnWorldEnd()
    {
        UnloadAllScene();
        for (size_t i = 0; i < m_deferredDestroyedQueue.size(); i++)
        {
            DestroyObject(m_deferredDestroyedQueue[i]);
        }
        m_deferredDestroyedQueue.clear();

        m_worldDescriptorLayout.reset();
        m_worldDescriptorBuffer.reset();
        m_worldDescriptors.reset();
    }

    void World::OnSceneLoading(ObjectPtr<Scene> scene)
    {
    }

    void World::OnSceneUnloading(ObjectPtr<Scene> scene)
    {
    }

} // namespace pulsar
