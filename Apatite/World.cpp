#include "World.h"
#include <Apatite/World.h>
#include <Apatite/Scene.h>

namespace apatite
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

    void World::Tick(float dt)
    {
        Ticker ticker;
        ticker.deltatime = dt;

        if (IsValid(this->scene))
        {
            for (auto& node : *this->scene->get_root_nodes())
            {
                if (node->IsAlive())
                {
                    node->OnTick(ticker);
                }
            }
        }


    }

    void World::Draw()
    {
        for (auto& node : *this->scene->get_root_nodes())
        {
            node->Draw();
        }
        
    }

    void World::OnWorldBegin()
    {
    }

    void World::OnWorldEnd()
    {
    }


}

