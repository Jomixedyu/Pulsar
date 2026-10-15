#pragma once
#include <Pulsar/Rendering/RenderGraph/Features/RenderFeatureSettings.h>

namespace pulsar
{
    class DisplayEncodingRenderFeatureSettings : public RenderFeatureSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::DisplayEncodingRenderFeatureSettings, RenderFeatureSettings);
    public:
        RenderFeatureFactory BuildRenderFeatureFactory() const override;
    };
}
