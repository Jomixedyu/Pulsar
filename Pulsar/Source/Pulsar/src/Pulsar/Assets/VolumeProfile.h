#pragma once
#include "Pulsar/AssetObject.h"
#include "Pulsar/Assets/VolumeSettings.h"
#include "Pulsar/Assets/TonemappingSettings.h"
#include "Pulsar/Assets/DisplayEncodingSettings.h"
#include "Pulsar/Assets/PostProcessMaterialSettings.h"

namespace pulsar
{
    class VolumeProfile : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::VolumeProfile, AssetObject);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute);

    public:
        VolumeProfile();

        void Serialize(AssetSerializer* s) override;
        void OnCollectAssetDependencies(array_list<guid_t>& deps) override;

        const List_sp<SPtr<VolumeSettings>>& GetEffects() const { return m_effects; }
        List_sp<SPtr<VolumeSettings>>& GetEffects() { return m_effects; }

    protected:
        CORELIB_REFL_DECL_FIELD(m_effects, new ListItemAttribute(cltypeof<VolumeSettings>()));
        List_sp<SPtr<VolumeSettings>> m_effects;
    };

} // namespace pulsar
