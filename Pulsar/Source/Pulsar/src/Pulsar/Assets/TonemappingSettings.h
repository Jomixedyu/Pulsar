#pragma once
#include "Pulsar/Assets/VolumeSettings.h"

namespace pulsar
{
    class TonemappingSettings : public VolumeSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::TonemappingSettings, VolumeSettings);
    public:
        CORELIB_REFL_DECL_FIELD(m_enabled);
        bool m_enabled = true;

        CORELIB_REFL_DECL_FIELD(m_mode);
        TonemappingMode m_mode = TonemappingMode::ACES;

        bool IsEnabled() const override { return m_enabled; }
        void Blend(float weight, VolumeSettings* accumulator) override;

    private:
        float m_blendWeight = 0.0f; // transient: dominant weight seen so far
    };

} // namespace pulsar
