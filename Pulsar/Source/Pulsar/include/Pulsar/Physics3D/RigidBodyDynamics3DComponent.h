#pragma once
#include "Components/Component.h"
#include "Physics3D/PhysicsWorld3D.h"
#include "Components/Shape3DComponent.h"
#include "Simulate.h"

namespace pulsar
{

    class RigidBodyDynamics3DComponent : public Component, public ISimulate
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
        array_list<Shape3DComponent_ref> CollectAttachedShapes() const;

    protected:
        Physics3DObject* m_physics;
    };
    DECL_PTR(RigidBodyDynamics3DComponent);
}