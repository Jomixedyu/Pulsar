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
        auto shapes = CollectAttachedShapes();

        if (shapes.empty())
        {
            return;
        }
        auto shapeType = shapes[0]->GetShapeType();

        Physics3DObject::ShapeType targetShapeType{};
        switch (shapeType)
        {
        case Shape3DType::Sphere:
            targetShapeType = Physics3DObject::SPHERE;
            break;
        case Shape3DType::Box:
            targetShapeType = Physics3DObject::BOX;
            break;
        case Shape3DType::Capsule:
            targetShapeType = Physics3DObject::CAPSULE;
            break;
        }

        m_physics = new Physics3DObject;
        m_physics->m_rigidMode = m_rigidMode;
        m_physics->m_shapeType = targetShapeType;
        m_physics->m_event = this;

        m_physics->m_boxHalfSize = shapes[0]->m_halfSize;
        m_physics->m_radius = shapes[0]->m_radius;

        auto transform = GetTransform();

        m_physics->m_position = transform->GetWorldPosition();
        m_physics->m_rotation = transform->GetRotation();

        GetWorld()->physicsWorld3D->AddObject(m_physics);
    }

    void RigidBodyDynamics3DComponent::EndSimulate()
    {
        GetWorld()->physicsWorld3D->RemoveObject(m_physics);
        delete m_physics;
        m_physics = nullptr;
    }

    void RigidBodyDynamics3DComponent::INotifyPhysics3DEvent_OnTransformChanged(Vector3f pos, Quat4f rot)
    {
        if (auto transform = GetTransform())
        {
            transform->SetWorldPosition(pos);
            transform->SetRotation(rot);
        }
    }

    array_list<Shape3DComponent_ref> RigidBodyDynamics3DComponent::CollectAttachedShapes() const
    {
        array_list<Shape3DComponent_ref> arr;
        GetNodePtr()->GetComponentsInChildren<Shape3DComponent>(arr);
        return arr;
    }

} // namespace pulsar