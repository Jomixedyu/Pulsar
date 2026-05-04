#include "Pulsar/Assets/TonemappingSettings.h"

namespace pulsar
{
    void TonemappingSettings::Blend(float weight, VolumeSettings* accumulator)
    {
        auto* acc = static_cast<TonemappingSettings*>(accumulator);
        if (!m_enabled || weight <= 0.0f)
            return;

        acc->m_enabled = true;
        if (weight > acc->m_blendWeight)
        {
            acc->m_mode = m_mode;
            acc->m_blendWeight = weight;
        }
    }
}
