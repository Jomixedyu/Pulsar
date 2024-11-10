#include "Components/BoxShape3DComponent.h"

namespace pulsar
{
    void BoxShape3DComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        Shape3DComponent::OnDrawGizmo(painter, selected);
    }
} // namespace pulsar