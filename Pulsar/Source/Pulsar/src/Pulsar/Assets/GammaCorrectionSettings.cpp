#include "Pulsar/Assets/GammaCorrectionSettings.h"

namespace pulsar
{
    void GammaCorrectionSettings::Blend(float weight, VolumeSettings* accumulator)
    {
        auto* acc = static_cast<GammaCorrectionSettings*>(accumulator);
        if (!m_enabled || weight <= 0.0f)
            return;

        acc->m_enabled = true;
        acc->m_blendWeight += weight;
        acc->m_gammaSum += m_gamma * weight;
        acc->m_gamma = acc->m_gammaSum / acc->m_blendWeight;
    }
}
