#pragma once
#include <Pulsar/Rendering/RenderGraph/Features/RenderFeatureSettings.h>

namespace pulsar
{
    class ColorGradingRenderFeatureSettings : public RenderFeatureSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ColorGradingRenderFeatureSettings, RenderFeatureSettings);
    public:
        RenderFeatureFactory BuildRenderFeatureFactory() const override;
    };
}
