#include <Apatite/World.h>

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
    }

    void World::OnWorldBegin()
    {
    }

    void World::OnWorldEnd()
    {
    }


}

