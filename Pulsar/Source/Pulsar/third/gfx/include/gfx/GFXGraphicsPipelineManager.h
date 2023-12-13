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
            const array_list<GFXDescriptorSetLayout_sp>& descriptorSetLayouts,
            const std::shared_ptr<GFXRenderPassLayout>& renderPass,
            const GFXGraphicsPipelineState& gpInfo) = 0;

        virtual void GCollect() = 0;

    };
}