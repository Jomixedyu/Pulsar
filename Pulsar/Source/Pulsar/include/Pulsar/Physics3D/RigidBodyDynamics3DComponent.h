#pragma once
#include "Components/Component.h"
#include "Physics3D/PhysicsWorld3D.h"
#include "Components/Shape3DComponent.h"
#include "Simulate.h"

namespace pulsar
{

    class RigidBodyDynamics3DComponent : public Component, public ISimulate, public INotifyPhysics3DEvent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RigidBodyDynamics3DComponent, Component);
        CORELIB_CLASS_ATTR(new CategoryAttribute("3D"));
    public:

        RigidBodyDynamics3DComponent();

        void BeginComponent() override;
        void EndComponent() override;

        void BeginSimulate() override;
        void EndSimulate() override;

    protected:
        void INotifyPhysics3DEvent_OnTransformChanged(Vector3f pos, Quat4f rot) override;

    protected:
        array_list<Shape3DComponent_ref> CollectAttachedShapes() const;

    protected:
        Physics3DObject* m_physics = nullptr;

        CORELIB_REFL_DECL_FIELD(m_rigidMode);
        RigidBody3DMode m_rigidMode{};

    };
    DECL_PTR(RigidBodyDynamics3DComponent);

}