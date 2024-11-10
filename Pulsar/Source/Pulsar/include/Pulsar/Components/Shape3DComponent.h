#pragma once
#include "Component.h"

namespace pulsar
{
    class Shape3DComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Shape3DComponent, Component);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);
    public:
        Shape3DComponent();

        CORELIB_REFL_DECL_FIELD(m_size);
        Vector3f m_size{};
        CORELIB_REFL_DECL_FIELD(m_radius);
        float m_radius{};
    };
    DECL_PTR(Shape3DComponent);
}