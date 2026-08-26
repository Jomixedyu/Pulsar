#pragma once
#include <Pulsar/Rendering/RenderGraph/Features/RenderFeatureSettings.h>

namespace pulsar
{
    class OpaqueRenderFeatureSettings : public RenderFeatureSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::OpaqueRenderFeatureSettings, RenderFeatureSettings);
    public:
        RenderFeatureFactory BuildRenderFeatureFactory() const override;
    };
}