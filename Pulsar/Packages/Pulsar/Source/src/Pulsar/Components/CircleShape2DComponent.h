#pragma once
#include "Shape2DComponent.h"

namespace pulsar
{
    class CircleShape2DComponent : public Shape2DComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CircleShape2DComponent, Shape2DComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("2D"));

    public:
        Shape2DType GetShapeType() override { return Shape2DType::Circle; }
        void OnDrawGizmo(GizmoPainter* painter, bool selected) override;
    };
} // namespace pulsar