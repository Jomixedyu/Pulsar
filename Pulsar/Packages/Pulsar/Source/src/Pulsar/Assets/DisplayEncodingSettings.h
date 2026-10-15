#pragma once
#include "Pulsar/Assets/VolumeSettings.h"

namespace pulsar
{
    class DisplayEncodingSettings;
    class DisplayEncodingRenderSnapshot;

    class DisplayEncodingSettings : public VolumeSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::DisplayEncodingSettings, VolumeSettings);
    public:
        CORELIB_REFL_DECL_FIELD(m_enabled);
        bool m_enabled = true;

        bool IsEnabled() const override { return m_enabled; }
        void Blend(float weight, VolumeSettings* accumulator) override;
        SPtr<VolumeRenderSnapshot> BuildRenderSnapshot() const override;
    };

    class DisplayEncodingRenderSnapshot final : public VolumeRenderSnapshot
    {
    public:
        using SettingsType = DisplayEncodingSettings;
        bool Enabled = false;

        Type* GetSettingsType() const override { return cltypeof<DisplayEncodingSettings>(); }
    };

} // namespace pulsar
