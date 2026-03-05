#include "Components/BoxShape2DComponent.h"

#include "Components/TransformComponent.h"

namespace pulsar
{

    void BoxShape2DComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        base::OnDrawGizmo(painter, selected);

        auto size = m_size;

        static StaticMeshVertex rect[8]{};

        rect[0].Position = Vector3f{-size.x, size.y, 0.f};
        rect[1].Position = Vector3f{size.x, size.y, 0.f};

        rect[2].Position = Vector3f{size.x, size.y, 0.f};
        rect[3].Position = Vector3f{size.x, -size.y, 0.f};

        rect[4].Position = Vector3f{size.x, -size.y, 0.f};
        rect[5].Position = Vector3f{-size.x, -size.y, 0.f};

        rect[6].Position = Vector3f{-size.x, -size.y, 0.f};
        rect[7].Position = Vector3f{-size.x, size.y, 0.f};

        auto color = selected ? GizmoPainter::DefaultSelectedLineColor : GizmoPainter::DefaultLineColor;

        for (auto& v : rect)
        {
            v.Position = GetTransform()->GetLocalToWorldMatrix() * v.Position;
            v.Color = color;
        }

        painter->DrawLines(rect, 8);
    }
} // namespace pulsar