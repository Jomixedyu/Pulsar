#include "Physics2D/RigidBodyDynamics2DComponent.h"

#include "Components/TransformComponent.h"
#include "Node.h"
#include "Physics2D/PhysicsWorld2D.h"
#include "World.h"

namespace pulsar
{
    RigidBodyDynamics2DComponent::RigidBodyDynamics2DComponent()
        : m_physics(nullptr)
    {
    }

    void RigidBodyDynamics2DComponent::BeginComponent()
    {
        base::BeginComponent();
        GetWorld()->GetSimulateManager().AddSimulate(this);
    }

    void RigidBodyDynamics2DComponent::EndComponent()
    {
        base::EndComponent();
        GetWorld()->GetSimulateManager().RemoveSimulate(this);
    }

    void RigidBodyDynamics2DComponent::BeginSimulate()
    {
        auto transform = GetTransform();
        auto pos = transform->GetWorldPosition();
        auto rot = transform->GetEuler();

        m_physics = new Physics2DObject;
        m_physics->m_rigidMode = m_mode;
        m_physics->m_position = Vector2f(pos.x, pos.y);
        m_physics->m_rotation = rot.z * math::deg2rad();
        m_physics->CallbackObject = this->GetObjectHandle();
        m_physics->OnTransformChanged = [this](Vector2f p, float r) {
            auto transform = GetTransform();
            auto oldPos = transform->GetPosition();
            transform->SetPosition(Vector3f(p.x, p.y, oldPos.z));
            auto euler = transform->GetEuler();
            euler.z = r * math::rad2deg();
            transform->SetEuler(euler);
        };
        m_physics->OnCollisionEnter = [this](ObjectHandle otherHandle) {
            auto otherComp = ObjectPtr<RigidBodyDynamics2DComponent>::UnsafeCreate(otherHandle);
            if (otherComp)
            {
                Collision2D collision{ otherComp->GetNode() };
                this->OnCollisionEnter2D.Invoke(collision);
            }
        };
        m_physics->OnCollisionExit = [this](ObjectHandle otherHandle) {
            auto otherComp = ObjectPtr<RigidBodyDynamics2DComponent>::UnsafeCreate(otherHandle);
            if (otherComp)
            {
                Collision2D collision{ otherComp->GetNode() };
                this->OnCollisionExit2D.Invoke(collision);
            }
        };
        m_physics->OnOverlapEnter = [this](ObjectHandle otherHandle) {
            auto otherComp = ObjectPtr<RigidBodyDynamics2DComponent>::UnsafeCreate(otherHandle);
            if (otherComp)
            {
                Collision2D collision{ otherComp->GetNode() };
                this->OnOverlapEnter2D.Invoke(collision);
            }
        };
        m_physics->OnOverlapExit = [this](ObjectHandle otherHandle) {
            auto otherComp = ObjectPtr<RigidBodyDynamics2DComponent>::UnsafeCreate(otherHandle);
            if (otherComp)
            {
                Collision2D collision{ otherComp->GetNode() };
                this->OnOverlapExit2D.Invoke(collision);
            }
        };

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
            shape.m_density = comp->GetDensity();
            shape.m_friction = comp->GetFriction();
            shape.m_isSensor = comp->GetIsSensor();
        }

        GetWorld()->GetPhysicsWorld2D()->AddObject(m_physics);
    }

    void RigidBodyDynamics2DComponent::EndSimulate()
    {
        GetWorld()->GetPhysicsWorld2D()->RemoveObject(m_physics);
        delete m_physics;
        m_physics = nullptr;
    }

    void RigidBodyDynamics2DComponent::SimulateTick(float dt)
    {
        if (!m_physics || m_mode == RigidBody2DMode::Dynamic)
            return;

        auto transform = GetTransform();
        auto pos = transform->GetWorldPosition();
        auto euler = transform->GetEuler();
        GetWorld()->GetPhysicsWorld2D()->SetTransform(
            m_physics,
            Vector2f(pos.x, pos.y),
            euler.z * math::deg2rad()
        );
    }

    void RigidBodyDynamics2DComponent::OnAttachedShapeChanged(Shape2DComponent* shape)
    {
    }

    Vector2f RigidBodyDynamics2DComponent::GetLinearVelocity() const
    {
        if (!m_physics) return {};
        return GetWorld()->GetPhysicsWorld2D()->GetLinearVelocity(m_physics);
    }

    void RigidBodyDynamics2DComponent::SetLinearVelocity(Vector2f velocity)
    {
        if (!m_physics) return;
        GetWorld()->GetPhysicsWorld2D()->SetLinearVelocity(m_physics, velocity);
    }

    void RigidBodyDynamics2DComponent::ApplyLinearImpulse(Vector2f impulse)
    {
        if (!m_physics) return;
        GetWorld()->GetPhysicsWorld2D()->ApplyLinearImpulse(m_physics, impulse, Vector2f(0,0));
    }

    void RigidBodyDynamics2DComponent::ApplyLinearImpulse(Vector2f impulse, Vector2f point)
    {
        if (!m_physics) return;
        GetWorld()->GetPhysicsWorld2D()->ApplyLinearImpulse(m_physics, impulse, point);
    }

    void RigidBodyDynamics2DComponent::SetTransform(Vector2f position, float rotation)
    {
        if (!m_physics) return;
        GetWorld()->GetPhysicsWorld2D()->SetTransform(m_physics, position, rotation);
    }

    array_list<Shape2DComponent_ref> RigidBodyDynamics2DComponent::CollectAttachedShapes() const
    {
        array_list<Shape2DComponent_ref> ret;
        GetNode()->GetComponentsInChildren<Shape2DComponent>(ret);
        return ret;
    }

} // namespace pulsar