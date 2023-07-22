#include "World.h"
#include <Pulsar/World.h>
#include <Pulsar/Scene.h>

namespace pulsar
{
    static World* _world_inst;

    World* World::Current()
    {
        return _world_inst;
    }

    World* World::Reset(World* world)
    {
        if (_world_inst)
        {
            _world_inst->OnWorldEnd();
        }
        _world_inst = world;
        if (_world_inst)
        {
            _world_inst->OnWorldBegin();
        }
        return _world_inst;
    }
    World::World()
    {
        m_scenes.push_back(Scene::StaticCreate("PresistentScene"));
    }

    void World::Tick(float dt)
    {
        Ticker ticker;
        ticker.deltatime = dt;

        for (auto& scene : m_scenes)
        {
            if (IsValid(scene))
            {
                for (auto& node : *scene->GetRootNodes())
                {
                    if (node->IsAlive())
                    {
                        node->OnTick(ticker);
                    }
                }
            }
        }

    }
    void World::ChangeScene(sptr<Scene> scene, bool clearPresistentScene)
    {
        if (clearPresistentScene)
        {
            m_scenes.clear();
        }
        else
        {
            auto presistent = m_scenes[0];
            m_scenes.clear();
            m_scenes.push_back(presistent);
        }
        m_scenes.push_back(scene);
    }

    void World::OnWorldBegin()
    {
    }

    void World::OnWorldEnd()
    {
    }


}

