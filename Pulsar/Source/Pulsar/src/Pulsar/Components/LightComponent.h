#pragma once
#include "Component.h"
#include "Pulsar/Rendering/LightingData.h"

namespace pulsar
{
    class LightComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::LightComponent, Component);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);
    public:
        void SetIntensity(float value);
        float GetIntensity() const { return m_intensity; }

        void SetColor(Color4f value);
        Color4f GetColor() const { return m_lightColor; }

    protected:

        void MarkRenderingDirty();

        void PostEditChange(FieldInfo* info) override;
        virtual void OnIntensityChanged() {};
        virtual void OnLightColorChanged() {};
    protected:
        CORELIB_REFL_DECL_FIELD(m_intensity);
        float m_intensity{1};
        CORELIB_REFL_DECL_FIELD(m_lightColor);
        Color4f m_lightColor{1,1,1,1};

        LightShaderParameter m_runtimeLightData{};
    };
}