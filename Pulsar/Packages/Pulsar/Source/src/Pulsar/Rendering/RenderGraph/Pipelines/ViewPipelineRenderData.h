#pragma once
#include <Pulsar/Rendering/RenderGraph/Features/RenderFeatureSettings.h>
#include <vector>

namespace pulsar
{
    struct ViewPipelineRenderData
    {
        std::vector<Type*> FeatureTypes;
        std::vector<RenderFeatureFactory> Features;
    };
}
