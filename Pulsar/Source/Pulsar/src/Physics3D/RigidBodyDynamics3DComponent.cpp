#include "Physics3D/RigidBodyDynamics3DComponent.h"

#include "World.h"

namespace pulsar
{

    RigidBodyDynamics3DComponent::RigidBodyDynamics3DComponent()
    {
    }
    void RigidBodyDynamics3DComponent::BeginComponent()
    {
        base::BeginComponent();
        GetWorld()->GetSimulateManager().AddSimulate(this);
    }
    void RigidBodyDynamics3DComponent::EndComponent()
    {
        base::EndComponent();
        GetWorld()->GetSimulateManager().AddSimulate(this);
    }
    void RigidBodyDynamics3DComponent::BeginSimulate()
    {
        m_physics = new Physics3DObject;

        GetWorld()->physicsWorld3D->AddObject(m_physics);
    }
    void RigidBodyDynamics3DComponent::EndSimulate()
    {
        GetWorld()->physicsWorld3D->RemoveObject(m_physics);
        delete m_physics;
        m_physics = nullptr;
    }
    array_list<Shape3DComponent_ref> RigidBodyDynamics3DComponent::CollectAttachedShapes() const
    {
        return {};
    }
} // namespace pulsar