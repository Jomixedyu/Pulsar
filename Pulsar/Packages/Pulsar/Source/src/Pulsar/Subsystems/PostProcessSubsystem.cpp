#include "Subsystems/PostProcessSubsystem.h"
#include "Pulsar/Components/VolumeComponent.h"
#include "Pulsar/Components/SceneCaptureComponent.h"
#include "Pulsar/Assets/VolumeProfile.h"
#include "Pulsar/Assets/ColorGradingSettings.h"
#include "Pulsar/World.h"
#include <unordered_map>
#include <algorithm>

namespace pulsar
{

    SUBSYSTEM_DECL(PostProcessSubsystem);

    void PostProcessSubsystem::OnBeginPlay()
    {
        // 不要在这里 clear m_volumes。
        // VolumeComponent 在 BeginComponent/EndComponent 中自己注册/注销，
        // PIE 场景复制后组件已经注册好了，clear 会把它们丢掉。
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
                stack.AddComponent(pair.second->BuildRenderSnapshot());
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

    void PostProcessSubsystem::NotifyVolumeSettingsChanged()
    {
        if (!m_world)
            return;

        for (auto* capture : m_world->GetCaptureManager().GetCaptures())
        {
            if (capture)
                capture->MarkPostProcessDirty();
        }
    }

} // namespace pulsar
