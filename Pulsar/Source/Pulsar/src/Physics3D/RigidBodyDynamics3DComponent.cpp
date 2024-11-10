#include "Physics3D/RigidBodyDynamics3DComponent.h"

#include "Node.h"
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
        GetWorld()->GetSimulateManager().RemoveSimulate(this);
    }

    void RigidBodyDynamics3DComponent::BeginSimulate()
    {
        m_physics = new Physics3DObject;
        m_physics->m_rigidMode = m_rigidMode;
        m_physics->m_shapeType = Physics3DObject::BOX;

        auto shapes = CollectAttachedShapes();

        m_physics->m_boxSize = shapes[0]->m_size;
        m_physics->m_radius = shapes[0]->m_radius;

        auto Transform = GetTransform();

        m_physics->m_position = Transform->GetWorldPosition();
        m_physics->m_rotation = Transform->GetRotation();

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
        array_list<Shape3DComponent_ref> arr;
        GetNodePtr()->GetComponentsInChildren<Shape3DComponent>(arr);
        return arr;
    }

} // namespace pulsar