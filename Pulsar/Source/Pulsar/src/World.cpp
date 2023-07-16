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
        m_scene = Scene::StaticCreate();
    }

    void World::Tick(float dt)
    {
        Ticker ticker;
        ticker.deltatime = dt;

        if (IsValid(m_scene))
        {
            for (auto& node : *m_scene->get_root_nodes())
            {
                if (node->IsAlive())
                {
                    node->OnTick(ticker);
                }
            }
        }


    }
    void World::ChangeScene(sptr<Scene> scene)
    {
        m_scene = scene;
    }

    void World::OnWorldBegin()
    {
    }

    void World::OnWorldEnd()
    {
    }


}

