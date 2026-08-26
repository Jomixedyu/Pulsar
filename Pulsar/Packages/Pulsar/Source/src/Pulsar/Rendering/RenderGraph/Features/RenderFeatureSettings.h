#pragma once
#include <Pulsar/Assembly.h>
#include <CoreLib/Object.h>
#include <CoreLib/Guid.h>
#include <Pulsar/Rendering/RenderGraph/Features/RenderFeature.h>
#include <functional>
#include <memory>

namespace pulsar
{
    using RenderFeatureFactory = std::function<std::unique_ptr<RenderFeature>()>;

    class RenderFeatureSettings : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RenderFeatureSettings, Object);

    public:
        virtual RenderFeatureFactory BuildRenderFeatureFactory() const { return {}; }
        virtual void CollectAssetDependencies(array_list<guid_t>& dependencies) {}
    };
}