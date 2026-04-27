#pragma once
#include "Pulsar/Assets/VolumeSettings.h"

namespace pulsar
{
    class GammaCorrectionSettings : public VolumeSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::GammaCorrectionSettings, VolumeSettings);
    public:
        CORELIB_REFL_DECL_FIELD(m_enabled);
        bool m_enabled = true;

        CORELIB_REFL_DECL_FIELD(m_gamma);
        float m_gamma = 2.2f;

        bool IsEnabled() const override { return m_enabled; }
        void Blend(float weight, VolumeSettings* accumulator) override;

    private:
        float m_blendWeight = 0.0f; // transient
        float m_gammaSum    = 0.0f; // transient
    };

} // namespace pulsar
