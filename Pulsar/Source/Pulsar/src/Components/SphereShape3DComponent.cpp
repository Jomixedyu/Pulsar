#include "Components/SphereShape3DComponent.h"

#include "Components/TransformComponent.h"
#include "Rendering/SimplePrimitiveUtils.h"
#include "TransformUtil.h"

namespace pulsar
{
    void SphereShape3DComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        base::OnDrawGizmo(painter, selected);

        static array_list<Vector3f> pos = SimplePrimitiveUtils::CreateSphere<Vector3f>(16);

        if (selected)
        {
            array_list<StaticMeshVertex> verts;
            verts.reserve(pos.size());
            auto matrix = GetTransform()->GetLocalToWorldMatrix();

            for (auto p : pos)
            {
                auto& vert = verts.emplace_back();
                vert.Color = painter->DefaultSelectedLineColor;
                vert.Position = matrix * (p * m_radius);
            }
            painter->DrawLineArray(verts);
        }

    }
} // namespace pulsar