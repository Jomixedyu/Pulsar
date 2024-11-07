#pragma once
#include "Subsystem.h"

namespace pulsar
{
    class WorldSubsystem : public Subsystem
    {
        friend class World;
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::WorldSubsystem, Subsystem);
    public:
        virtual void OnBeginPlay() {}
        virtual void OnEndPlay() {}
        virtual void OnTick(float dt) {}

        World* GetWorld() const { return m_world; }

    protected:
        World* m_world = nullptr;
    };
} // namespace pulsar