#pragma once
#include "Simulate.h"
#include "Components/Shape2DComponent.h"

#include <Pulsar/Components/Component.h>
#include <Pulsar/Physics2D/PhysicsWorld2D.h>

namespace pulsar
{
    class Physics2DObject;

    class RigidBodyDynamics2DComponent : public Component, public ISimulate
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RigidBodyDynamics2DComponent, Component);
        CORELIB_CLASS_ATTR(new CategoryAttribute("2D"));
    public:

        RigidBodyDynamics2DComponent();

        void BeginComponent() override;
        void EndComponent() override;

        void BeginSimulate() override;
        void EndSimulate() override;
        void SimulateTick(float dt) override;

        void OnAttachedShapeChanged(Shape2DComponent* shape);

        RigidBody2DMode GetMode() const { return m_mode; }
        void SetMode(RigidBody2DMode mode) { m_mode = mode; }

        Vector2f GetLinearVelocity() const;
        void SetLinearVelocity(Vector2f velocity);
        void ApplyLinearImpulse(Vector2f impulse);
        void ApplyLinearImpulse(Vector2f impulse, Vector2f point);
        void SetTransform(Vector2f position, float rotation);
    protected:
        array_list<Shape2DComponent_ref> CollectAttachedShapes() const;
        uint32_t ComputeShapeVersion() const;
        void RebuildPhysicsObject();
    public:
        Action<const Collision2D&> OnCollisionEnter2D;
        Action<const Collision2D&> OnCollisionExit2D;
        Action<const Collision2D&> OnOverlapEnter2D;
        Action<const Collision2D&> OnOverlapExit2D;
    protected:
        uint32_t m_lastShapeVersion = 0;

        // CORELIB_REFL_DECL_FIELD(m_density);
        // float m_density;
        // CORELIB_REFL_DECL_FIELD(m_friction);
        // float m_friction;
        // CORELIB_REFL_DECL_FIELD(m_restitution);
        // float m_restitution;
        // CORELIB_REFL_DECL_FIELD(m_isSensor);
        // bool m_isSensor;
        CORELIB_REFL_DECL_FIELD(m_mode);
        RigidBody2DMode m_mode{};

        class Physics2DObject* m_physics;
    };
    DECL_PTR(RigidBodyDynamics2DComponent);

}