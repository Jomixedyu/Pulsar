#include "Gameplay/Character2d.h"
#include "Logger.h"
#include "Node.h"
#include "World.h"

namespace pulsar
{
    Character2d::Character2d()
    {
    }

    void Character2d::BeginPlay()
    {
        base::BeginPlay();
        m_targetHorizontalVelocity = 0.0f;
        m_verticalVelocity = 0.0f;
        EnsureRigidbody();
    }

    void Character2d::EndPlay()
    {
        if (m_rigidbody && m_subscribed)
        {
            m_rigidbody->OnCollisionEnter2D.RemoveListenerByInstance(this);
            m_rigidbody->OnCollisionExit2D.RemoveListenerByInstance(this);
        }
        base::EndPlay();
    }

    void Character2d::EnsureRigidbody()
    {
        if (m_subscribed) return;
        m_rigidbody = GetNode()->GetComponent<RigidBodyDynamics2DComponent>().GetPtr();
        if (!m_rigidbody) return;

        m_rigidbody->OnCollisionEnter2D.AddListener(this, &Character2d::OnCollisionEnter);
        m_rigidbody->OnCollisionExit2D.AddListener(this, &Character2d::OnCollisionExit);
        m_subscribed = true;
    }

    void Character2d::OnCollisionEnter(const Collision2D& collision)
    {
        if (!collision.node) return;
        m_groundContactCount++;
        m_isGrounded = true;
    }

    void Character2d::OnCollisionExit(const Collision2D& collision)
    {
        if (!collision.node) return;
        m_groundContactCount = std::max(0, m_groundContactCount - 1);
        m_isGrounded = m_groundContactCount > 0;
    }

    void Character2d::Move(float direction)
    {
        EnsureRigidbody();
        if (!m_rigidbody) return;

        if (direction > 0.001f) m_facingDirection = 1.0f;
        if (direction < -0.001f) m_facingDirection = -1.0f;

        m_targetHorizontalVelocity = direction * m_speed;

        if (m_rigidbody->GetMode() == RigidBody2DMode::Dynamic)
        {
            auto vel = m_rigidbody->GetLinearVelocity();
            vel.x = m_targetHorizontalVelocity;
            m_rigidbody->SetLinearVelocity(vel);
        }
    }

    void Character2d::Jump()
    {
        EnsureRigidbody();
        if (!m_rigidbody || !m_isGrounded) return;

        if (m_rigidbody->GetMode() == RigidBody2DMode::Dynamic)
        {
            auto vel = m_rigidbody->GetLinearVelocity();
            vel.y = m_jumpImpulse;
            m_rigidbody->SetLinearVelocity(vel);
        }
        else
        {
            m_verticalVelocity = m_jumpImpulse;
        }

        m_isGrounded = false;
        m_groundContactCount = 0;
    }

    void Character2d::Land()
    {
        m_verticalVelocity = 0.0f;
        m_isGrounded = true;
        m_groundContactCount = std::max(m_groundContactCount, 1);
    }

    Vector2f Character2d::GetVelocity() const
    {
        if (!m_rigidbody) return {};
        if (m_rigidbody->GetMode() == RigidBody2DMode::Dynamic)
            return m_rigidbody->GetLinearVelocity();
        return Vector2f(m_targetHorizontalVelocity, m_verticalVelocity);
    }

    void Character2d::OnTick(Ticker ticker)
    {
        base::OnTick(ticker);
        EnsureRigidbody();
        if (!m_rigidbody) return;

        if (m_rigidbody->GetMode() != RigidBody2DMode::Dynamic)
        {
            float dt = ticker.deltatime;

            if (!m_isGrounded)
            {
                m_verticalVelocity += m_gravity * dt;
            }
            else if (m_verticalVelocity < 0.0f)
            {
                m_verticalVelocity = 0.0f;
            }

            Vector2f vel(m_targetHorizontalVelocity, m_verticalVelocity);
            m_rigidbody->SetLinearVelocity(vel);
        }
    }
}
