#include "Components/Circle2DComponent.h"

#include "Components/TransformComponent.h"

namespace pulsar
{
    static void CreateCircleLines(array_list<StaticMeshVertex>& verts, int segmentCount, float radius, Color4f color)
    {
        auto theta = jmath::Radians(360.f / (float)segmentCount);
        for (int i = 0; i < segmentCount; ++i)
        {
            auto& p1 = verts.emplace_back();
            p1.Position = Vector3f{cos(theta * i), sin(theta * i), 0} * radius;
            p1.Color = color;
            auto& p2 = verts.emplace_back();
            p2.Position = Vector3f{cos(theta * float(i + 1)), sin(theta * float(i + 1)), 0} * radius;
            p2.Color = color;
        }
    }

    void Circle2DComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        base::OnDrawGizmo(painter, selected);
        auto pos = GetTransform()->GetWorldPosition();
        auto mat = Translate(pos);
        auto color = selected ? GizmoPainter::DefaultSelectedLineColor : GizmoPainter::DefaultLineColor;

        array_list<StaticMeshVertex> verts;
        CreateCircleLines(verts, 64, m_radius, color);

        for (auto& vert : verts)
        {
            vert.Position = mat * vert.Position;
            vert.Color = color;
        }

        painter->DrawLineArray(verts);

    }
} // namespace pulsar