#pragma once
#include "WorldSubsystem.h"
#include "Pulsar/Assets/VolumeProfile.h"

namespace pulsar
{
    struct BlendedPostProcessSettings
    {
        bool  enabled = false;       // post-processing is off unless a volume enables it
        bool  hasTonemapping = false;
        int   tonemappingMode = 1;   // default: ACES
        float gamma = 2.2f;
        bool  applyGamma = false;
    };

    class PostProcessSubsystem : public WorldSubsystem
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::PostProcessSubsystem, WorldSubsystem);

    public:
        void OnBeginPlay() override;
        void OnEndPlay() override;

        void RegisterVolume(class VolumeComponent* volume);
        void UnregisterVolume(class VolumeComponent* volume);

        // Query blended post-process settings for a given world position.
        BlendedPostProcessSettings QuerySettings(const Vector3f& worldPos) const;

        // Query all active volumes affecting the given world position.
        array_list<class VolumeComponent*> QueryVolumes(const Vector3f& worldPos) const;

        // Query all post-process materials from active volumes at the given world position.
        // Materials are collected in volume priority order.
        array_list<RCPtr<Material>> QueryPostProcessMaterials(const Vector3f& worldPos) const;

    protected:
        array_list<class VolumeComponent*> m_volumes;
    };

} // namespace pulsar
