#include "Pulsar/Assets/BloomSettings.h"

namespace pulsar
{
    void BloomSettings::Blend(float weight, VolumeSettings* accumulator)
    {
        auto* acc = static_cast<BloomSettings*>(accumulator);
        if (!m_enabled || weight <= 0.0f)
            return;

        acc->m_enabled = true;
        acc->m_threshold = m_threshold * weight + acc->m_threshold * (1.0f - weight);
        acc->m_intensity = m_intensity * weight + acc->m_intensity * (1.0f - weight);
    }

    SPtr<VolumeRenderSnapshot> BloomSettings::BuildRenderSnapshot() const
    {
        auto snapshot = mksptr(new BloomRenderSnapshot());
        snapshot->Enabled = m_enabled;
        snapshot->Threshold = m_threshold;
        snapshot->Intensity = m_intensity;
        return snapshot;
    }
}
