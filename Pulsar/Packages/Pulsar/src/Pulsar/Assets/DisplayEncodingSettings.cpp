#include "Pulsar/Assets/DisplayEncodingSettings.h"

namespace pulsar
{
    void DisplayEncodingSettings::Blend(float weight, VolumeSettings* accumulator)
    {
        auto* acc = static_cast<DisplayEncodingSettings*>(accumulator);
        if (!m_enabled || weight <= 0.0f)
            return;

        acc->m_enabled = true;
    }
}
