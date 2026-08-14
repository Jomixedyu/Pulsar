#pragma once
#include <Pulsar/Rendering/RenderGraph/Features/RenderFeatureRenderData.h>
#include <memory>
#include <vector>

namespace pulsar
{
    struct ViewPipelineRenderData
    {
        std::vector<std::shared_ptr<RenderFeatureRenderData>> Features;
    };
}