#include "World.h"
#include <Pulsar/World.h>
#include <Pulsar/Scene.h>
#include <Pulsar/Logger.h>

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
    World::World()
    {
        
    }

    World::~World()
    {

    }

    void World::Tick(float dt)
    {
        Ticker ticker{};
        ticker.deltatime = dt;

        for (auto& scene : m_scenes)
        {
            if (IsValid(scene))
            {
                for (auto& node : *scene->GetNodes())
                {
                    if (IsValid(node) && node->GetIsActive())
                    {
                        node->OnTick(ticker);
                    }
                }
            }
        }

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
        auto it = std::find(m_scenes.begin(), m_scenes.end(), scene);
        if (it != m_scenes.end())
        {
            if (it == m_scenes.begin())
            {
                OnUnloadingPersistentScene(scene);
            }
            OnSceneUnloading(scene);
            scene->EndScene();
            m_scenes.erase(it);
            DestroyObject(scene, true);
        }
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

    void World::AddRenderObject(const rendering::RenderObject_sp renderObject)
    {
        renderObject->OnCreateResource();
        m_renderObjects.insert(renderObject);
    }
    void World::RemoveRenderObject(rendering::RenderObject_rsp renderObject)
    {
        auto it = m_renderObjects.find(renderObject);
        if (it != m_renderObjects.end())
        {
            (*it)->OnDestroyResource();
            m_renderObjects.erase(it);
        }
    }


    void World::OnWorldBegin()
    {
        InitializePersistentScene();
    }

    void World::OnWorldEnd()
    {
        UnloadAllScene();
        for (size_t i = 0; i < m_deferredDestroyedQueue.size(); i++)
        {
            DestroyObject(m_deferredDestroyedQueue[i]);
        }
        m_deferredDestroyedQueue.clear();
    }

    void World::OnSceneLoading(ObjectPtr<Scene> scene)
    {
    }

    void World::OnSceneUnloading(ObjectPtr<Scene> scene)
    {
    }


}

