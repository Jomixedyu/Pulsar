#include "Pulsar/Assets/PostProcessMaterialSettings.h"

namespace pulsar
{
    void PostProcessMaterialSettings::Blend(float weight, VolumeSettings* accumulator)
    {
        // Post-process materials are handled separately via QueryPostProcessMaterials().
        // No numeric blending is performed here.
    }

    void PostProcessMaterialSettings::CollectAssetDependencies(array_list<guid_t>& deps)
    {
        for (auto& mat : *m_materials)
        {
            if (mat)
            {
                auto guid = mat.GetGuid();
                if (!guid.is_empty())
                    deps.push_back(guid);
            }
        }
    }
}
