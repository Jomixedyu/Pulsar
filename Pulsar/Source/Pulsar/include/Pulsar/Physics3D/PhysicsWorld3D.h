#pragma once
#include "Assembly.h"

namespace pulsar
{

    class _PhysicsWorld3DNative;

    class Physics3DObject
    {

    };

    class PhysicsWorld3D
    {
    public:
        void BeginSimulate();
        void EndSimulate();
        void StepSimulate(float dt);

        void AddObject(Physics3DObject* object);
        void RemoveObject(Physics3DObject* object);
    protected:
        _PhysicsWorld3DNative* m_world = nullptr;
        array_list<Physics3DObject*> m_objects;
    };

}