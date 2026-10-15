#pragma once
#include <Pulsar/Rendering/RenderGraph/Features/RenderFeatureSettings.h>
#include <Pulsar/Assets/Material.h>

namespace pulsar
{
    class FullScreenRenderFeatureSettings : public RenderFeatureSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::FullScreenRenderFeatureSettings, RenderFeatureSettings);
    public:
        RenderFeatureFactory BuildRenderFeatureFactory() const override;
        void CollectAssetDependencies(array_list<guid_t>& dependencies) override;

        CORELIB_REFL_DECL_FIELD(m_material);
        RCPtr<Material> m_material;
    };
}
