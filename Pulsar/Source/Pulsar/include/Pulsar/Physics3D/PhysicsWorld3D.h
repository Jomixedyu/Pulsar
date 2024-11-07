#pragma once
#include "Assembly.h"

namespace pulsar
{

    class _PhysicsWorld3DNative;

    class PhysicsWorld3D
    {
    public:
        void BeginSimulate();
        void EndSimulate();
        void StepSimulate(float dt);

    protected:
        _PhysicsWorld3DNative* m_world = nullptr;
    };

}