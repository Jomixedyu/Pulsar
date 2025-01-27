#pragma once
#include "Shape3DComponent.h"

namespace pulsar
{
    class CapsuleShape3DComponent : public Shape3DComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CapsuleShape3DComponent, Shape3DComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("3D"));
    public:
        Shape3DType GetShapeType() const override { return Shape3DType::Capsule; }
    };
} // namespace pulsar