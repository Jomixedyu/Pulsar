#include "Components/BoxShape3DComponent.h"

#include "Components/TransformComponent.h"
#include "Rendering/SimplePrimitiveUtils.h"

namespace pulsar
{

    void BoxShape3DComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        static auto array = SimplePrimitiveUtils::CreateBox<Vector3f>();
        if (selected)
        {
            StaticMeshVertex vertices[24];

            auto local2World = GetTransform()->GetLocalToWorldMatrix();
            auto color = selected ? GizmoPainter::DefaultSelectedLineColor : GizmoPainter::DefaultLineColor;

            for (int i = 0; i < 24; ++i)
            {
                vertices[i].Position = local2World * (array[i] * m_halfSize);
                vertices[i].Color = color;
            }

            painter->DrawLines(vertices, 24);
        }
    }

} // namespace pulsar