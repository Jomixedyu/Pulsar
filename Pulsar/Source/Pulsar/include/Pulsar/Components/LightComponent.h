#pragma once
#include "Component.h"

namespace pulsar
{
    class LightComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::LightComponent, Component);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);
    public:

    protected:
        CORELIB_REFL_DECL_FIELD(m_intensity);
        float m_intensity{};
        CORELIB_REFL_DECL_FIELD(m_lightColor);
        Color4f m_lightColor;
    };
}