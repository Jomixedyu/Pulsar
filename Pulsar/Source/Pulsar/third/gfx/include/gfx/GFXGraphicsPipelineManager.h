#pragma once
#include "GFXGraphicsPipeline.h"
#include "GFXInclude.h"
#include "GFXShaderPass.h"
#include "GFXRenderPass.h"

namespace gfx
{
    
    class GFXGraphicsPipelineManager
    {
    public:
        GFXGraphicsPipelineManager() {}
        GFXGraphicsPipelineManager(const GFXGraphicsPipelineManager&) = delete;
        GFXGraphicsPipelineManager(GFXGraphicsPipelineManager&&) = delete;
        virtual ~GFXGraphicsPipelineManager() {}
    public:
        virtual std::shared_ptr<GFXGraphicsPipeline> GetGraphicsPipeline(
            const std::shared_ptr<GFXShaderPass>& shaderPass,
            const std::shared_ptr<GFXRenderPassLayout>& renderPass) = 0;

        virtual void GCollect() = 0;

    };
}