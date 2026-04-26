#pragma once
#include "Component.h"

namespace pulsar
{

    enum class Shape3DType
    {
        Sphere,
        Box,
        Capsule,
    };
    class Shape3DComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Shape3DComponent, Component);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);
    public:
        Shape3DComponent();

        virtual Shape3DType GetShapeType() const = 0;

        CORELIB_REFL_DECL_FIELD(m_halfSize);
        Vector3f m_halfSize {0.5f, 0.5f, 0.5f};

        CORELIB_REFL_DECL_FIELD(m_radius);
        float m_radius { 0.5f };
    };
    DECL_PTR(Shape3DComponent);
}