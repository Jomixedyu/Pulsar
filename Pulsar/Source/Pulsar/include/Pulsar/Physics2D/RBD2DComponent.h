#pragma once
#include "Simulate.h"
#include "Components/Shape2DComponent.h"

#include <Pulsar/Components/Component.h>
#include <Pulsar/Physics2D/PhysicsWorld2D.h>

namespace pulsar
{
    class Physics2DObject;

    struct Collision2D
    {
        ObjectPtr<Node> node;
    };

    class RBD2DComponent : public Component, public ISimulate, public INotifyPhysics2DEvent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RBD2DComponent, Component);
        CORELIB_CLASS_ATTR(new CategoryAttribute("2D"));
    public:

        RBD2DComponent();

        void BeginComponent() override;
        void EndComponent() override;

        void BeginSimulate() override;
        void EndSimulate() override;

        void INotifyPhysics2DEvent_OnChangedTransform(Vector2f pos, float rot) override;

        void OnAttachedShapeChanged(Shape2DComponent* shape);
    protected:
        array_list<Shape2DComponent_ref> CollectAttachedShapes() const;
    public:
        ActionEvents<const Collision2D&> OnCollisionEnter2D;
        ActionEvents<const Collision2D&> OnCollisionExit2D;
        ActionEvents<const Collision2D&> OnOverlap2D;
        ActionEvents<const Collision2D&> OnExitEnter2D;
    protected:

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
    DECL_PTR(RBD2DComponent);

}