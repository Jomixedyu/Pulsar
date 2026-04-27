#include "Subsystems/PostProcessSubsystem.h"
#include "Pulsar/Components/VolumeComponent.h"
#include "Pulsar/Assets/VolumeProfile.h"
#include "Pulsar/Assets/ColorGradingSettings.h"
#include "Pulsar/Assets/PostProcessMaterialSettings.h"
#include <unordered_map>
#include <algorithm>

namespace pulsar
{

    SUBSYSTEM_DECL(PostProcessSubsystem);

    void PostProcessSubsystem::OnBeginPlay()
    {
        m_volumes.clear();
    }

    void PostProcessSubsystem::OnEndPlay()
    {
        m_volumes.clear();
    }

    void PostProcessSubsystem::RegisterVolume(VolumeComponent* volume)
    {
        if (!volume) return;
        m_volumes.push_back(volume);
    }

    void PostProcessSubsystem::UnregisterVolume(VolumeComponent* volume)
    {
        if (!volume) return;
        auto it = std::find(m_volumes.begin(), m_volumes.end(), volume);
        if (it != m_volumes.end())
            m_volumes.erase(it);
    }

    VolumeStack PostProcessSubsystem::QuerySettings(const Vector3f& worldPos) const
    {
        std::unordered_map<Type*, SPtr<VolumeSettings>> accumulators;

        for (auto* vol : m_volumes)
        {
            if (!vol || !vol->GetProfile())
                continue;

            float weight = vol->ComputeBlendWeight(worldPos);
            if (weight <= 0.0f)
                continue;

            auto* profile = vol->GetProfile().GetPtr();
            if (!profile)
                continue;

            for (auto& effect : *profile->GetEffects())
            {
                if (!effect || !effect->SupportsBlending() || !effect->IsEnabled())
                    continue;

                Type* effectType = effect->GetType();
                auto it = accumulators.find(effectType);
                if (it == accumulators.end())
                {
                    auto acc = std::static_pointer_cast<VolumeSettings>(effectType->CreateSharedInstance({}));
                    if (acc)
                    {
                        effect->Blend(weight, acc.get());
                        accumulators[effectType] = acc;
                    }
                }
                else
                {
                    effect->Blend(weight, it->second.get());
                }
            }
        }

        VolumeStack stack;
        for (auto& pair : accumulators)
        {
            if (pair.second)
                stack.AddComponent(pair.first, pair.second);
        }
        return stack;
    }

    array_list<VolumeComponent*> PostProcessSubsystem::QueryVolumes(const Vector3f& worldPos) const
    {
        array_list<VolumeComponent*> result;
        for (auto* vol : m_volumes)
        {
            if (!vol) continue;
            float weight = vol->ComputeBlendWeight(worldPos);
            if (weight > 0.0f)
                result.push_back(vol);
        }
        return result;
    }

    array_list<RCPtr<Material>> PostProcessSubsystem::QueryPostProcessMaterials(const Vector3f& worldPos) const
    {
        array_list<RCPtr<Material>> result;

        array_list<VolumeComponent*> sortedVolumes;
        for (auto* vol : m_volumes)
        {
            if (!vol) continue;
            if (vol->ComputeBlendWeight(worldPos) > 0.0f)
                sortedVolumes.push_back(vol);
        }

        std::sort(sortedVolumes.begin(), sortedVolumes.end(),
            [](VolumeComponent* a, VolumeComponent* b)
            {
                return a->GetPriority() < b->GetPriority();
            });

        for (auto* vol : sortedVolumes)
        {
            auto* profile = vol->GetProfile().GetPtr();
            if (!profile)
                continue;
            for (auto& effect : *profile->GetEffects())
            {
                if (auto* ppMat = dynamic_cast<PostProcessMaterialSettings*>(effect.get()))
                {
                    for (auto& mat : *ppMat->m_materials)
                    {
                        if (mat)
                            result.push_back(mat);
                    }
                }
            }
        }

        return result;
    }

} // namespace pulsar
