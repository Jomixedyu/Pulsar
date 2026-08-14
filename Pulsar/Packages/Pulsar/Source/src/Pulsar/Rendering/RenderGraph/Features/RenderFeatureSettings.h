#pragma once
#include <Pulsar/Assembly.h>
#include <CoreLib/Object.h>
#include <CoreLib/Guid.h>
#include <memory>

namespace pulsar
{
    class RenderFeatureRenderData;

    class RenderFeatureSettings : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RenderFeatureSettings, Object);

    public:
        virtual std::shared_ptr<RenderFeatureRenderData> BuildRenderData() const { return nullptr; }
        virtual void CollectAssetDependencies(array_list<guid_t>& dependencies) {}
    };
}