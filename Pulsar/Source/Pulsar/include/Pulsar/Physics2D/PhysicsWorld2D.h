#pragma once
#include "Assembly.h"
#include "Pulsar/EngineMath.h"

namespace pulsar
{
    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar, RigidBody2DMode,
        Static,
        Dynamic,
        Kinematic);
}
CORELIB_DECL_BOXING(pulsar::RigidBody2DMode, pulsar::BoxingRigidBody2DMode);

namespace pulsar
{
    class PhysicsWorld2D;

    class INotifyPhysics2DEvent
    {
    public:
        virtual ~INotifyPhysics2DEvent() = default;
        virtual void INotifyPhysics2DEvent_OnChangedTransform(Vector2f pos, float rot) = 0;
    };

    class Physics2DObject
    {
    public:
        Physics2DObject() = default;
        virtual ~Physics2DObject() = default;

        enum ShapeType { BOX, CIRCLE, CAPSULE };
        struct Shape
        {
            ShapeType type{};
            Vector2f size {0.5f, 0.5f};
            float radius = 0.5f;
            float m_density = 1.f;
            float m_friction = 0.6f;
            bool m_isSensor = false;
        };

        INotifyPhysics2DEvent* m_event{};
        RigidBody2DMode m_rigidMode{};
        Vector2f m_position{};

        array_list<Shape> m_shapes;
    protected:
        PhysicsWorld2D* m_world = nullptr;
    };


    class _PhysicsWorld2DNative;

    class PhysicsWorld2D
    {
    public:

        void Tick(float dt);

        void BeginSimulate();
        void EndSimulate();

        PhysicsWorld2D();

        void AddObject(Physics2DObject* object);
        void RemoveObject(Physics2DObject* object);

    protected:
        bool m_isSimulating = false;

        array_list<Physics2DObject*> m_objects;
        _PhysicsWorld2DNative* m_world = nullptr;
    };
}