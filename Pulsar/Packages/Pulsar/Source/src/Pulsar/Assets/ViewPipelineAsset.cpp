#include "ViewPipelineAsset.h"

namespace pulsar
{
    ViewPipelineAsset::ViewPipelineAsset()
    {
        init_sptr_member(m_features);
    }

    void ViewPipelineAsset::Serialize(AssetSerializer* serializer)
    {
        base::Serialize(serializer);

        if (serializer->IsWrite)
        {
            auto features = serializer->Object->New(ser::VarientType::Array);
            for (const auto& feature : *m_features)
                features->Push(feature ? feature->GetAssetGuid().to_string() : "");
            serializer->Object->Add("Features", features);
            return;
        }

        m_features->clear();
        auto features = serializer->Object->At("Features");
        if (!features)
            return;

        for (int index = 0; index < features->GetCount(); ++index)
        {
            auto guid = guid_t::parse(features->At(index)->AsString());
            m_features->push_back(RuntimeAssetManager::GetLoadedAssetByGuid<RenderFeatureAsset>(guid));
        }
    }

    void ViewPipelineAsset::OnCollectAssetDependencies(array_list<guid_t>& dependencies)
    {
        for (const auto& feature : *m_features)
        {
            if (feature)
                dependencies.push_back(feature->GetAssetGuid());
        }
    }

    void ViewPipelineAsset::BuildRenderData(ViewPipelineRenderData& outData) const
    {
        outData.Features.clear();
        for (const auto& feature : *m_features)
        {
            if (!feature)
                continue;

            auto renderData = feature->BuildRenderData();
            if (renderData)
                outData.Features.push_back(std::move(renderData));
        }
    }
}