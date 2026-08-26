#include "ViewPipelineSettings.h"
#include <CoreLib/Assembly.h>
#include <CoreLib.Serialization/JsonSerializer.h>

namespace pulsar
{
    ViewPipelineSettings::ViewPipelineSettings()
    {
        init_sptr_member(m_features);
    }

    void ViewPipelineSettings::Serialize(AssetSerializer* serializer)
    {
        base::Serialize(serializer);

        ser::JsonSerializerSettings settings{};
        settings.SaveObjectType = true;

        if (serializer->IsWrite)
        {
            auto featuresArray = serializer->Object->New(ser::VarientType::Array);
            for (const auto& feature : *m_features)
            {
                if (!feature)
                    continue;

                auto featureObject = featuresArray->New(ser::VarientType::Object);
                auto featureJson = ser::JsonSerializer::Serialize(feature.get(), settings);
                featureObject->AssignParse(featureJson);
                featuresArray->Push(featureObject);
            }
            serializer->Object->Add("Features", featuresArray);
            return;
        }

        m_features->clear();
        auto featuresArray = serializer->Object->At("Features");
        if (!featuresArray)
            return;

        for (int index = 0; index < featuresArray->GetCount(); ++index)
        {
            auto featureObject = featuresArray->At(index);
            if (!featureObject)
                continue;

            auto feature = sptr_cast<RenderFeatureSettings>(
                ser::JsonSerializer::Deserialize(featureObject->ToString(), cltypeof<RenderFeatureSettings>()));
            if (feature)
                m_features->push_back(feature);
        }
    }

    void ViewPipelineSettings::OnCollectAssetDependencies(array_list<guid_t>& dependencies)
    {
        for (const auto& feature : *m_features)
        {
            if (feature)
                feature->CollectAssetDependencies(dependencies);
        }
    }

    void ViewPipelineSettings::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        NotifyModified();
    }

    void ViewPipelineSettings::GetSubscribeObserverHandles(array_list<ObjectHandle>& out)
    {
        base::GetSubscribeObserverHandles(out);
    }

    void ViewPipelineSettings::OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg)
    {
        base::OnNotifyObserver(inDependency, msg);
        if (EnumHasFlag(msg, DependencyObjectState::Modified))
            NotifyModified();
    }

    void ViewPipelineSettings::NotifyModified()
    {
        RebuildObserver();
        RuntimeObjectManager::NotifyDependencySource(GetObjectHandle(), DependencyObjectState::Modified);
    }

    void ViewPipelineSettings::BuildRenderData(ViewPipelineRenderData& outData) const
    {
        outData.FeatureTypes.clear();
        outData.Features.clear();
        for (const auto& feature : *m_features)
        {
            if (!feature)
                continue;

            auto factory = feature->BuildRenderFeatureFactory();
            if (factory)
            {
                outData.FeatureTypes.push_back(feature->GetType());
                outData.Features.push_back(std::move(factory));
            }
        }
    }
}
