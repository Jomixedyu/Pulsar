#pragma once
#include "Components/Component.h"
#include "Physics2D/RigidBodyDynamics2DComponent.h"

namespace pulsar
{
    class Character2d : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Character2d, Component);
        CORELIB_CLASS_ATTR(new CategoryAttribute("2D"))
    public:
        Character2d();

        void BeginPlay() override;
        void EndPlay() override;
        void OnTick(Ticker ticker) override;

        void Move(float direction);
        void Jump();
        void Land();

        bool IsGrounded() const { return m_isGrounded; }
        float GetFacingDirection() const { return m_facingDirection; }
        Vector2f GetVelocity() const;

    protected:
        void EnsureRigidbody();
        void OnCollisionEnter(const Collision2D& collision);
        void OnCollisionExit(const Collision2D& collision);

        CORELIB_REFL_DECL_FIELD(m_speed);
        float m_speed = 5.0f;

        CORELIB_REFL_DECL_FIELD(m_jumpImpulse);
        float m_jumpImpulse = 8.0f;

        CORELIB_REFL_DECL_FIELD(m_gravity);
        float m_gravity = -20.0f;

        RigidBodyDynamics2DComponent* m_rigidbody = nullptr;
        bool m_subscribed = false;

        float m_facingDirection = 1.0f;
        bool m_isGrounded = false;
        int m_groundContactCount = 0;

        float m_targetHorizontalVelocity = 0.0f;
        float m_verticalVelocity = 0.0f;
    };
}
