#include "Components/BoxShape3DComponent.h"

#include "Components/TransformComponent.h"
#include "TransformUtil.h"

namespace pulsar
{

    void BoxShape3DComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        if (selected)
        {
            Matrix4f scale{1};
            transutil::NewScale(scale, m_halfSize);
            auto matrix = GetTransform()->GetLocalToWorldMatrix() * scale;
            painter->DrawWireCube(matrix, GizmoPainter::DefaultSelectedLineColor);
        }
    }

} // namespace pulsar