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
    };

} // namespace pulsar
