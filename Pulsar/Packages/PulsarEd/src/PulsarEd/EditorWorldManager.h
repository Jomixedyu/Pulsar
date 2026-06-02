#pragma once
#include "Assembly.h"
#include "Pulsar/World.h"

namespace pulsared
{
    class EditorWorldManager final
    {
    public:
        static World* GetWorld();
        static void SetWorld(World* world);

    };
}