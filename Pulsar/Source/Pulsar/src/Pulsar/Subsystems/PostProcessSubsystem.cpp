#include "Subsystems/PostProcessSubsystem.h"
#include "Pulsar/Components/VolumeComponent.h"
#include "Pulsar/Assets/VolumeProfile.h"

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

    BlendedPostProcessSettings PostProcessSubsystem::QuerySettings(const Vector3f& worldPos) const
    {
        struct EffectWeights
        {
            float tonemappingWeight = 0.0f;
            int   dominantMode = 1;   // ACES
            float dominantModeWeight = 0.0f;
            bool  hasTonemapping = false;

            bool  applyGamma = false;
            float gammaWeight = 0.0f;
            float gammaSum = 0.0f;
        };

        EffectWeights acc;
        float totalEffectWeight = 0.0f;

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
                if (!effect)
                    continue;

                if (auto* tm = dynamic_cast<TonemappingSettings*>(effect.get()))
                {
                    if (tm->m_enabled)
                    {
                        acc.tonemappingWeight += weight;
                        totalEffectWeight += weight;
                        acc.hasTonemapping = true;

                        if (weight > acc.dominantModeWeight)
                        {
                            acc.dominantMode = static_cast<int>(tm->m_mode);
                            acc.dominantModeWeight = weight;
                        }
                    }
                }
                else if (auto* gamma = dynamic_cast<GammaCorrectionSettings*>(effect.get()))
                {
                    if (gamma->m_enabled)
                    {
                        acc.gammaWeight += weight;
                        acc.gammaSum += gamma->m_gamma * weight;
                        totalEffectWeight += weight;
                        acc.applyGamma = true;
                    }
                }
            }
        }

        BlendedPostProcessSettings result{};
        result.gamma = 2.2f;

        if (totalEffectWeight > 0.0f)
        {
            result.enabled = true;

            result.hasTonemapping = acc.hasTonemapping;

            if (acc.tonemappingWeight > 0.0f)
            {
                result.tonemappingMode = acc.dominantMode;
            }

            if (acc.gammaWeight > 0.0f)
            {
                result.gamma = acc.gammaSum / acc.gammaWeight;
                result.applyGamma = acc.applyGamma;
            }
        }

        return result;
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
