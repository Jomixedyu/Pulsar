#include "Gameplay/Character2d.h"
#include "Logger.h"
#include "Node.h"
#include "World.h"
#include "Physics2D/RigidBodyDynamics2DComponent.h"
#include "Components/TransformComponent.h"
#include "Gizmos.h"
#include <cmath>

namespace pulsar
{
    Character2d::Character2d()
    {
        m_canDrawGizmo = true;
    }

    void Character2d::BeginPlay()
    {
        base::BeginPlay();
        m_velocity = Vector2f(0, 0);
        m_isGrounded = false;
        m_groundNode = nullptr;
    }

    Vector2f Character2d::Move(Vector2f motion)
    {
        auto pos3 = GetNode()->GetTransform()->GetPosition();
        Vector2f pos(pos3.x, pos3.y);
        auto physics = GetWorld()->GetPhysicsWorld2D();

        if (!physics)
        {
            pos += motion;
            GetNode()->GetTransform()->SetPosition(Vector3f(pos.x, pos.y, pos3.z));
            return motion;
        }

        Vector2f actualMotion = motion;

        // skinWidth 必须大于 0，否则 Box2D shape cast 从精确接触面开始时
        // 浮点误差会导致偶发性地忽略碰撞体，造成穿墙/穿地
        const float effectiveSkinWidth = std::max(m_skinWidth, 0.001f);

        // --- 垂直方向 ---
        if (std::abs(actualMotion.y) > 0.0001f)
        {
            PhysicsWorld2D::CapsuleCastResult result{};
            if (physics->CastCapsule(pos, m_radius, m_height, Vector2f(0, actualMotion.y), result))
            {
                float maxMove = actualMotion.y * result.fraction;
                if (actualMotion.y > 0)
                    maxMove = std::max(0.0f, maxMove - effectiveSkinWidth);
                else
                    maxMove = std::min(0.0f, maxMove + effectiveSkinWidth);
                actualMotion.y = maxMove;

                auto comp = ObjectPtr<RigidBodyDynamics2DComponent>::UnsafeCreate(result.hitObject).GetPtr();
                auto hitNode = comp ? comp->GetNode() : nullptr;
                OnControllerColliderHit.Invoke(Collision2D{ hitNode });
            }
        }

        pos.y += actualMotion.y;

        // --- 水平方向 ---
        if (std::abs(actualMotion.x) > 0.0001f)
        {
            PhysicsWorld2D::CapsuleCastResult result{};
            bool hit = physics->CastCapsule(pos, m_radius, m_height, Vector2f(actualMotion.x, 0), result);
            if (hit)
            {
                bool stepped = false;

                // 尝试上台阶
                if (m_stepOffset > 0.0001f)
                {
                    float charBottomY = pos.y - (m_height * 0.5f - m_radius);
                    float hitHeight = result.point.y - charBottomY;

                    if (hitHeight > 0 && hitHeight <= m_stepOffset)
                    {
                        // 1. 先检测向上抬升是否安全
                        PhysicsWorld2D::CapsuleCastResult upResult{};
                        if (!physics->CastCapsule(pos, m_radius, m_height, Vector2f(0, m_stepOffset), upResult))
                        {
                            // 2. 在抬升后的位置检测水平移动是否安全
                            Vector2f stepUpPos = pos + Vector2f(0, m_stepOffset);
                            PhysicsWorld2D::CapsuleCastResult stepResult{};
                            if (!physics->CastCapsule(stepUpPos, m_radius, m_height, Vector2f(actualMotion.x, 0), stepResult))
                            {
                                // 上台阶成功
                                pos.x += actualMotion.x;
                                pos.y += m_stepOffset;
                                actualMotion.y += m_stepOffset;
                                stepped = true;
                            }
                        }
                    }
                }

                if (!stepped)
                {
                    // 普通阻挡
                    float maxMove = actualMotion.x * result.fraction;
                    if (actualMotion.x > 0)
                        maxMove = std::max(0.0f, maxMove - effectiveSkinWidth);
                    else
                        maxMove = std::min(0.0f, maxMove + effectiveSkinWidth);
                    actualMotion.x = maxMove;
                    pos.x += maxMove;

                    auto comp = ObjectPtr<RigidBodyDynamics2DComponent>::UnsafeCreate(result.hitObject).GetPtr();
                    auto hitNode = comp ? comp->GetNode() : nullptr;
                    OnControllerColliderHit.Invoke(Collision2D{ hitNode });
                }
            }
            else
            {
                pos.x += actualMotion.x;
            }
        }

        GetNode()->GetTransform()->SetPosition(Vector3f(pos.x, pos.y, pos3.z));

        // 更新朝向（根据水平速度方向翻转）
        if (actualMotion.x > 0.001f) m_facingDirection = 1.0f;
        if (actualMotion.x < -0.001f) m_facingDirection = -1.0f;
        auto scale = GetNode()->GetTransform()->GetScale();
        scale.x = m_facingDirection;
        GetNode()->GetTransform()->SetScale(scale);

        UpdateGrounded();

        return actualMotion;
    }

    void Character2d::UpdateGrounded()
    {
        auto physics = GetWorld()->GetPhysicsWorld2D();
        if (!physics) return;

        auto pos3 = GetNode()->GetTransform()->GetPosition();
        Vector2f pos(pos3.x, pos3.y);

        bool wasGrounded = m_isGrounded;

        // 向下发射短距离 capsule cast 检测地面
        // 从稍微上方开始 cast，避免起始点已和地面重叠导致被 Box2D 忽略
        const float effectiveSkinWidth = std::max(m_skinWidth, 0.001f);
        float checkStartOffset = 0.02f;
        float checkDist = effectiveSkinWidth + 0.03f;
        Vector2f checkStart = pos + Vector2f(0, checkStartOffset);
        PhysicsWorld2D::CapsuleCastResult result{};
        if (physics->CastCapsule(checkStart, m_radius, m_height, Vector2f(0, -(checkDist + checkStartOffset)), result))
        {
            // 检查坡度限制（法线 y 分量越大越平）
            float slopeAngle = std::acos(std::clamp(result.normal.y, -1.0f, 1.0f)) * 57.29578f;
            if (slopeAngle <= m_slopeLimit)
            {
                if (!wasGrounded)
                {
                    m_isGrounded = true;
                    auto comp = ObjectPtr<RigidBodyDynamics2DComponent>::UnsafeCreate(result.hitObject).GetPtr();
                    m_groundNode = comp ? comp->GetNode() : nullptr;
                    OnCollisionEnter.Invoke(Collision2D{ m_groundNode });
                }
                else
                {
                    m_isGrounded = true;
                }
                return;
            }
        }

        if (wasGrounded)
        {
            m_isGrounded = false;
            OnCollisionExit.Invoke(Collision2D{ m_groundNode });
            m_groundNode = nullptr;
        }
    }

    void Character2d::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        base::OnDrawGizmo(painter, selected);

        auto color = selected ? GizmoPainter::DefaultSelectedLineColor : GizmoPainter::DefaultLineColor;
        auto matrix = GetTransform()->GetLocalToWorldMatrix();

        float halfLine = m_height * 0.5f - m_radius;
        if (halfLine < 0) halfLine = 0;

        constexpr int segments = 16;
        array_list<StaticMeshVertex> points;
        points.reserve(segments * 2 + 4);

        auto AddLinePoint = [&](const Vector3f& localPos)
        {
            StaticMeshVertex v{};
            v.Position = matrix * localPos;
            v.Color = color;
            points.push_back(v);
        };

        // 左侧竖线
        AddLinePoint(Vector3f(-m_radius, -halfLine, 0));
        AddLinePoint(Vector3f(-m_radius, halfLine, 0));

        // 右侧竖线
        AddLinePoint(Vector3f(m_radius, -halfLine, 0));
        AddLinePoint(Vector3f(m_radius, halfLine, 0));

        // 顶部半圆（从左侧到右侧，经过上方）
        for (int i = 0; i < segments / 2; ++i)
        {
            float angle1 = 3.14159265f - (3.14159265f * i) / (segments / 2);
            float angle2 = 3.14159265f - (3.14159265f * (i + 1)) / (segments / 2);

            float x1 = std::cos(angle1) * m_radius;
            float y1 = std::sin(angle1) * m_radius + halfLine;
            float x2 = std::cos(angle2) * m_radius;
            float y2 = std::sin(angle2) * m_radius + halfLine;

            AddLinePoint(Vector3f(x1, y1, 0));
            AddLinePoint(Vector3f(x2, y2, 0));
        }

        // 底部半圆（从右侧到左侧，经过下方）
        for (int i = 0; i < segments / 2; ++i)
        {
            float angle1 = -(3.14159265f * i) / (segments / 2);
            float angle2 = -(3.14159265f * (i + 1)) / (segments / 2);

            float x1 = std::cos(angle1) * m_radius;
            float y1 = std::sin(angle1) * m_radius - halfLine;
            float x2 = std::cos(angle2) * m_radius;
            float y2 = std::sin(angle2) * m_radius - halfLine;

            AddLinePoint(Vector3f(x1, y1, 0));
            AddLinePoint(Vector3f(x2, y2, 0));
        }

        painter->DrawLineArray(points);
    }
}
