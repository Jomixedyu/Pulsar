#include "Components/VolumeComponent.h"
#include "Pulsar/Node.h"
#include "Pulsar/TransformUtil.h"
#include "Pulsar/Subsystems/PostProcessSubsystem.h"
#include "Pulsar/World.h"
#include "Pulsar/Gizmos.h"
#include "Pulsar/Rendering/SimplePrimitiveUtils.h"
#include <CoreLib.Math/Jmath.h>

namespace pulsar
{

    VolumeComponent::VolumeComponent()
    {
        m_canDrawGizmo = true;
    }

    void VolumeComponent::GetDependenciesAsset(array_list<guid_t>& deps) const
    {
        base::GetDependenciesAsset(deps);
        if (m_profile)
        {
            deps.push_back(m_profile.GetGuid());
        }
    }

    void VolumeComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        static auto boxArray = SimplePrimitiveUtils::CreateBox<Vector3f>();

        auto local2World = GetTransform()->GetLocalToWorldMatrix();
        auto extent = GetEffectiveExtent();
        auto color = selected ? GizmoPainter::DefaultSelectedLineColor : GizmoPainter::DefaultLineColor;

        StaticMeshVertex vertices[24];
        for (int i = 0; i < 24; ++i)
        {
            vertices[i].Position = local2World * (boxArray[i] * extent);
            vertices[i].Color = color;
        }
        painter->DrawLines(vertices, 24);

        // Draw blend distance outline (fainter, larger box)
        if (m_blendDistance > 0.0f && !m_isGlobal)
        {
            auto blendExtent = extent + Vector3f(m_blendDistance);
            StaticMeshVertex blendVerts[24];
            auto blendColor = Color4b{color.r, color.g, color.b, uint8_t(color.a * 0.4f)};
            for (int i = 0; i < 24; ++i)
            {
                blendVerts[i].Position = local2World * (boxArray[i] * blendExtent);
                blendVerts[i].Color = blendColor;
            }
            painter->DrawLines(blendVerts, 24);
        }
    }

    void VolumeComponent::BeginComponent()
    {
        base::BeginComponent();
        if (auto* world = GetWorld())
        {
            if (auto* ppSub = world->GetSubsystem<PostProcessSubsystem>())
                ppSub->RegisterVolume(this);
        }
    }

    void VolumeComponent::EndComponent()
    {
        if (auto* world = GetWorld())
        {
            if (auto* ppSub = world->GetSubsystem<PostProcessSubsystem>())
                ppSub->UnregisterVolume(this);
        }
        base::EndComponent();
    }

    Vector3f VolumeComponent::GetEffectiveExtent() const
    {
        auto node = GetNode();
        if (!node)
            return Vector3f(0.5f);

        auto transform = node->GetTransform();
        if (!transform)
            return Vector3f(0.5f);

        // Use the world scale as the box half-extent.
        return transform->GetWorldScale() * 0.5f;
    }

    float VolumeComponent::ComputeBlendWeight(const Vector3f& worldPos) const
    {
        if (m_isGlobal)
            return 1.0f;

        auto node = GetNode();
        if (!node)
            return 0.0f;

        auto transform = node->GetTransform();
        if (!transform)
            return 0.0f;

        // Transform world position into the volume's local space.
        Vector4f localPos4 = transform->GetWorldToLocalMatrix() * Vector4f(worldPos, 1.0f);
        Vector3f localPos = localPos4.xyz();

        Vector3f extent = GetEffectiveExtent();

        // Signed distance to box boundary (positive = outside).
        Vector3f d = jmath::Abs(localPos) - extent;
        float maxDist = jmath::MaxComponent(jmath::Max(d, Vector3f(0.0f)));

        // If fully inside, weight = 1.0.
        if (maxDist <= 0.0f)
            return 1.0f;

        // Hard cut if no blend distance.
        if (m_blendDistance <= 0.0f)
            return 0.0f;

        // Smooth blend from boundary outward.
        float t = maxDist / m_blendDistance;
        return std::max(0.0f, 1.0f - t);
    }

} // namespace pulsar
