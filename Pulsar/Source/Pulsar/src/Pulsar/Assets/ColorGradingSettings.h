#pragma once
#include "VolumeSettings.h"
#include <Pulsar/Assets/Texture2D.h>

namespace pulsar
{
    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar, LUTColorSpace, Linear, LogC, sRGB);
}

CORELIB_DECL_BOXING(pulsar::LUTColorSpace, pulsar::BoxingLUTColorSpace);

namespace pulsar
{
    class ColorGradingSettings : public VolumeSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ColorGradingSettings, VolumeSettings);
    public:
        CORELIB_REFL_DECL_FIELD(m_enabled);
        bool m_enabled = true;

        CORELIB_REFL_DECL_FIELD(m_intensity);
        float m_intensity = 1.0f;

        CORELIB_REFL_DECL_FIELD(m_lutSize);
        int m_lutSize = 16;

        CORELIB_REFL_DECL_FIELD(m_colorSpace);
        LUTColorSpace m_colorSpace = LUTColorSpace::Linear;

        CORELIB_REFL_DECL_FIELD(m_lutTexture);
        RCPtr<Texture2D> m_lutTexture;

        bool IsEnabled() const override { return m_enabled; }
        void Blend(float weight, VolumeSettings* accumulator) override;

    private:
        float m_blendWeight       = 0.0f; // transient
        float m_intensitySum      = 0.0f; // transient
        float m_dominantLUTWeight = 0.0f; // transient
    };

} // namespace pulsar
