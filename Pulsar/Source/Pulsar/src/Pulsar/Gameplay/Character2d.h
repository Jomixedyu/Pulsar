#pragma once
#include "Components/Component.h"
#include "Physics2D/PhysicsWorld2D.h"

namespace pulsar
{
    /// Unity CharacterController 风格的 2D 手动角色控制器
    /// 不依赖 RigidBodyDynamics2DComponent，自己用 Capsule Cast 做碰撞检测
    class Character2d : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Character2d, Component);
        CORELIB_CLASS_ATTR(new CategoryAttribute("2D"))
    public:
        Character2d();

        void BeginPlay() override;

        /// 核心移动方法（类似 Unity CharacterController.Move）
        /// @param motion 本帧期望位移（已包含重力等）
        /// @return 实际位移
        Vector2f Move(Vector2f motion);

        bool IsGrounded() const { return m_isGrounded; }
        float GetFacingDirection() const { return m_facingDirection; }
        Vector2f GetVelocity() const { return m_velocity; }

        // 胶囊体参数（Inspector 可配置）
        CORELIB_REFL_DECL_FIELD(m_height);
        float m_height = 2.0f;      // 胶囊体总高度

        CORELIB_REFL_DECL_FIELD(m_radius);
        float m_radius = 0.5f;      // 胶囊体半径

        CORELIB_REFL_DECL_FIELD(m_stepOffset);
        float m_stepOffset = 0.3f;  // 自动上台阶高度

        CORELIB_REFL_DECL_FIELD(m_slopeLimit);
        float m_slopeLimit = 45.0f; // 斜坡角度限制（度）

        CORELIB_REFL_DECL_FIELD(m_skinWidth);
        float m_skinWidth = 0.01f;  // 皮肤宽度（允许微小重叠，防止抖动）

        Action<const Collision2D&> OnControllerColliderHit;
        Action<const Collision2D&> OnCollisionEnter;
        Action<const Collision2D&> OnCollisionExit;

        void OnDrawGizmo(GizmoPainter* painter, bool selected) override;

    protected:
        void UpdateGrounded();

        bool m_isGrounded = false;
        float m_facingDirection = 1.0f;
        Vector2f m_velocity{};
        ObjectPtr<Node> m_groundNode;
    };
}
