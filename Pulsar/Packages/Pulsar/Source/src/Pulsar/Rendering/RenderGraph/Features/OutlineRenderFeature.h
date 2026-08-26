#pragma once
#include <Pulsar/Rendering/RenderGraph/Features/RenderFeatureSettings.h>

namespace pulsar
{
    class OutlineRenderFeatureSettings : public RenderFeatureSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::OutlineRenderFeatureSettings, RenderFeatureSettings);
    public:
        RenderFeatureFactory BuildRenderFeatureFactory() const override;
    };
}