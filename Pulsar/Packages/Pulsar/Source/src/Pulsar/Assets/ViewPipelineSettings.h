#pragma once
#include <Pulsar/AssetObject.h>
#include <Pulsar/Rendering/RenderGraph/Features/RenderFeatureSettings.h>
#include <Pulsar/Rendering/RenderGraph/Pipelines/ViewPipelineRenderData.h>

namespace pulsar
{
    class ViewPipelineSettings : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ViewPipelineSettings, AssetObject);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute("Pulsar/Render Pipelines/View"));

    public:
        ViewPipelineSettings();

        void Serialize(AssetSerializer* serializer) override;
        void OnCollectAssetDependencies(array_list<guid_t>& dependencies) override;
        void PostEditChange(FieldInfo* info) override;
        void GetSubscribeObserverHandles(array_list<ObjectHandle>& out) override;
        void OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg) override;
        void NotifyModified();
        void BuildRenderData(ViewPipelineRenderData& outData) const;

        List_sp<SPtr<RenderFeatureSettings>>& GetFeatures() { return m_features; }
        const List_sp<SPtr<RenderFeatureSettings>>& GetFeatures() const { return m_features; }

    private:
        CORELIB_REFL_DECL_FIELD(m_features, new ListItemAttribute(cltypeof<RenderFeatureSettings>()));
        List_sp<SPtr<RenderFeatureSettings>> m_features;
    };
    DECL_PTR(ViewPipelineSettings);
}
