#pragma once
#include <Pulsar/AssetObject.h>
#include <memory>

namespace pulsar
{
    class RenderFeatureRenderData;

    class RenderFeatureAsset : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RenderFeatureAsset, AssetObject);

    public:
        virtual std::shared_ptr<RenderFeatureRenderData> BuildRenderData() const = 0;
    };
    DECL_PTR(RenderFeatureAsset);
}