#include "Physics2D/RBD2DComponent.h"

#include "Components/TransformComponent.h"
#include "Node.h"
#include "Physics2D/PhysicsWorld2D.h"
#include "World.h"

namespace pulsar
{
    RBD2DComponent::RBD2DComponent()
        : m_physics(nullptr)
    {
    }

    void RBD2DComponent::BeginComponent()
    {
        base::BeginComponent();
        GetWorld()->GetSimulateManager().AddSimulate(this);
    }

    void RBD2DComponent::EndComponent()
    {
        base::EndComponent();
        GetWorld()->GetSimulateManager().RemoveSimulate(this);
    }

    void RBD2DComponent::BeginSimulate()
    {
        auto transform = GetTransform();
        auto pos = transform->GetWorldPosition();
        auto rot = transform->GetEuler();

        m_physics = new Physics2DObject;
        m_physics->m_rigidMode = m_mode;
        m_physics->m_position = Vector2f(pos.x, pos.y);
        m_physics->m_rotation = rot.z * math::deg2rad();
        m_physics->m_event = this;

        for (auto& compRef : CollectAttachedShapes())
        {
            auto& shape = m_physics->m_shapes.emplace_back();
            const auto comp = compRef.GetPtr();

            switch (comp->GetShapeType())
            {
            case Shape2DType::Box:
                shape.type = Physics2DObject::BOX;
                break;
            case Shape2DType::Circle:
                shape.type = Physics2DObject::CIRCLE;
                break;
            case Shape2DType::Capsule:
                shape.type = Physics2DObject::CAPSULE;
                break;
            }

            shape.size = comp->GetSize();
            shape.radius = comp->GetRadius();
            shape.m_density = comp->m_density;
            shape.m_friction = comp->m_friction;
            shape.m_isSensor = comp->m_isSensor;
        }

        GetWorld()->physicsWorld2D->AddObject(m_physics);
    }

    void RBD2DComponent::EndSimulate()
    {
        GetWorld()->physicsWorld2D->RemoveObject(m_physics);
        delete m_physics;
        m_physics = nullptr;
    }

    void RBD2DComponent::INotifyPhysics2DEvent_OnChangedTransform(Vector2f pos, float rot)
    {
        auto transform = GetTransform();

        auto oldPos = transform->GetPosition();
        transform->SetPosition(Vector3f(pos.x, pos.y, oldPos.z));


        auto euler = transform->GetEuler();
        euler.z = rot * math::rad2deg();
        transform->SetEuler(euler);

    }

    void RBD2DComponent::OnAttachedShapeChanged(Shape2DComponent* shape)
    {
    }

    array_list<Shape2DComponent_ref> RBD2DComponent::CollectAttachedShapes() const
    {
        array_list<Shape2DComponent_ref> ret;
        GetNode()->GetComponentsInChildren<Shape2DComponent>(ret);
        return ret;
    }

} // namespace pulsar