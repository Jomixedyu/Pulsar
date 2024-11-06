#include "Components/Box2DComponent.h"

#include "Components/TransformComponent.h"

namespace pulsar
{
    void Box2DComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        base::OnDrawGizmo(painter, selected);

        auto pos = GetTransform()->GetWorldPosition();
        auto min = pos - Vector3f(m_size.x, m_size.y, 0);
        auto max = pos + Vector3f(m_size.x, m_size.y, 0);
        auto color = selected ? GizmoPainter::DefaultSelectedLineColor : GizmoPainter::DefaultLineColor;

        StaticMeshVertex vert[8];
        vert[0].Position = Vector3f(min.x, min.y, pos.z);
        vert[1].Position = Vector3f(max.x, min.y, pos.z);
        vert[2].Position = Vector3f(max.x, min.y, pos.z);
        vert[3].Position = Vector3f(max.x, max.y, pos.z);
        vert[4].Position = Vector3f(max.x, max.y, pos.z);
        vert[5].Position = Vector3f(min.x, max.y, pos.z);
        vert[6].Position = Vector3f(min.x, max.y, pos.z);
        vert[7].Position = Vector3f(min.x, min.y, pos.z);

        for (auto& v : vert)
        {
            v.Color = color;
        }

        painter->DrawLines(vert, 8);
    }
} // namespace pulsar