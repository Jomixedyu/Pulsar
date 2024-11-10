#pragma once
#include "Assembly.h"
#include "EngineMath.h"

namespace pulsar
{
    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar, RigidBody3DMode,
        Static,
        Dynamic,
        Kinematic);
}
CORELIB_DECL_BOXING(pulsar::RigidBody3DMode, pulsar::BoxingRigidBody3DMode);

namespace pulsar
{

    class _PhysicsWorld3DNative;

    class Physics3DObject
    {
    public:
        enum ShapeType { BOX, SPHERE, CAPSULE, MESH };

        RigidBody3DMode m_rigidMode{};
        ShapeType m_shapeType{};
        Vector3f m_boxSize{};
        float m_radius{};
        Vector3f m_position{};
        Quat4f m_rotation{};
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