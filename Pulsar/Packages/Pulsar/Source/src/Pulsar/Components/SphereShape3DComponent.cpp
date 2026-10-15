#include "Components/SphereShape3DComponent.h"

#include "Components/TransformComponent.h"
#include "Rendering/SimplePrimitiveUtils.h"
#include "TransformUtil.h"

namespace pulsar
{
    void SphereShape3DComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        base::OnDrawGizmo(painter, selected);

        if (selected)
        {
            Matrix4f scale{1};
            transutil::NewScale(scale, Vector3f{m_radius, m_radius, m_radius});
            auto matrix = GetTransform()->GetLocalToWorldMatrix() * scale;
            painter->DrawWireSphere(matrix, painter->DefaultSelectedLineColor);
        }
    }
} // namespace pulsar