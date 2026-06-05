#include "Pulsar/Assets/ColorGradingSettings.h"

namespace pulsar
{
    void ColorGradingSettings::Blend(float weight, VolumeSettings* accumulator)
    {
        auto* acc = static_cast<ColorGradingSettings*>(accumulator);
        if (!m_enabled || weight <= 0.0f)
            return;

        acc->m_enabled = true;
        acc->m_blendWeight += weight;
        acc->m_intensitySum += m_intensity * weight;
        acc->m_intensity = acc->m_intensitySum / acc->m_blendWeight;

        if (weight > acc->m_dominantLUTWeight)
        {
            acc->m_lutTexture = m_lutTexture;
            acc->m_lutSize = m_lutSize;
            acc->m_colorSpace = m_colorSpace;
            acc->m_dominantLUTWeight = weight;
        }
    }

    void ColorGradingSettings::CollectAssetDependencies(array_list<guid_t>& deps)
    {
        if (m_lutTexture)
        {
            auto guid = m_lutTexture.GetGuid();
            if (!guid.is_empty())
                deps.push_back(guid);
        }
    }
}
