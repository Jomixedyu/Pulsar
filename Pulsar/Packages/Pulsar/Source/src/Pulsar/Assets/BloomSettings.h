#pragma once
#include "Pulsar/Assets/VolumeSettings.h"

namespace pulsar
{
    class BloomSettings : public VolumeSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BloomSettings, VolumeSettings);
    public:
        CORELIB_REFL_DECL_FIELD(m_enabled);
        bool m_enabled = true;

        CORELIB_REFL_DECL_FIELD(m_threshold);
        float m_threshold = 0.44922f;

        CORELIB_REFL_DECL_FIELD(m_intensity);
        float m_intensity = 1.0f;

        bool IsEnabled() const override { return m_enabled; }
        void Blend(float weight, VolumeSettings* accumulator) override;

    private:
        float m_blendWeight = 0.0f;
    };
}
