#include "Pulsar/Assets/PostProcessMaterialSettings.h"

namespace pulsar
{
    void PostProcessMaterialSettings::Blend(float weight, VolumeSettings* accumulator)
    {
        // Post-process materials are handled separately via QueryPostProcessMaterials().
        // No numeric blending is performed here.
    }
}
