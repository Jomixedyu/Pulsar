#pragma once
#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/RenderFeatureAsset.h>
#include <Pulsar/Rendering/RenderGraph/Pipelines/ViewPipelineRenderData.h>

namespace pulsar
{
    class ViewPipelineAsset : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ViewPipelineAsset, AssetObject);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute("Pulsar/Render Pipelines/View"));

    public:
        ViewPipelineAsset();

        void Serialize(AssetSerializer* serializer) override;
        void OnCollectAssetDependencies(array_list<guid_t>& dependencies) override;
        void BuildRenderData(ViewPipelineRenderData& outData) const;

        List_sp<RCPtr<RenderFeatureAsset>>& GetFeatures() { return m_features; }
        const List_sp<RCPtr<RenderFeatureAsset>>& GetFeatures() const { return m_features; }

    private:
        CORELIB_REFL_DECL_FIELD(m_features, new ListItemAttribute(cltypeof<RenderFeatureAsset>()));
        List_sp<RCPtr<RenderFeatureAsset>> m_features;
    };
    DECL_PTR(ViewPipelineAsset);
}