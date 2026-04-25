#pragma once
#include "Pulsar/Assets/VolumeSettings.h"
#include "Pulsar/Assets/Material.h"
#include <Pulsar/Meta/PropertyStyleAttributes.h>

namespace pulsar
{
    class PostProcessMaterialSettings : public VolumeSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::PostProcessMaterialSettings, VolumeSettings);
    public:
        PostProcessMaterialSettings()
        {
            init_sptr_member(m_materials);
        }

        CORELIB_REFL_DECL_FIELD(m_materials, new ListItemAttribute(cltypeof<Material>()));
        List_sp<RCPtr<Material>> m_materials;
    };

} // namespace pulsar
