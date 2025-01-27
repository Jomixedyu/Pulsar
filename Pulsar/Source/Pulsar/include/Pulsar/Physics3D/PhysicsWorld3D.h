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

    class INotifyPhysics3DEvent
    {
    public:
        virtual ~INotifyPhysics3DEvent() = default;
        virtual void INotifyPhysics3DEvent_OnTransformChanged(Vector3f pos, Quat4f rot) = 0;
    };

    class Physics3DObject
    {
    public:
        enum ShapeType { BOX, SPHERE, CAPSULE, MESH };

        INotifyPhysics3DEvent* m_event = nullptr;
        RigidBody3DMode m_rigidMode{};
        ShapeType m_shapeType{};
        Vector3f m_boxHalfSize{};
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
        void AddObjectToSystem(Physics3DObject* object);
        void RemoveObjectFromSystem(Physics3DObject* object);

    protected:
        _PhysicsWorld3DNative* m_world = nullptr;
        array_list<Physics3DObject*> m_objects;
    };


} // namespace pulsar