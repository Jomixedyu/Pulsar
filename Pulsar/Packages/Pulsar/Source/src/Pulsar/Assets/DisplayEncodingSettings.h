#pragma once
#include "Pulsar/Assets/VolumeSettings.h"

namespace pulsar
{
    class DisplayEncodingSettings : public VolumeSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::DisplayEncodingSettings, VolumeSettings);
    public:
        CORELIB_REFL_DECL_FIELD(m_enabled);
        bool m_enabled = true;

        bool IsEnabled() const override { return m_enabled; }
        void Blend(float weight, VolumeSettings* accumulator) override;
    };

} // namespace pulsar
