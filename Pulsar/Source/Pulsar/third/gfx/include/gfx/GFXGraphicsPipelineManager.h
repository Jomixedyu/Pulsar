#pragma once
#include "GFXGraphicsPipeline.h"
#include "GFXInclude.h"
#include "GFXRenderPass.h"
#include "GFXGpuProgram.h"
#include "GFXDescriptorSet.h"

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
            const array_list<GFXGpuProgram_sp>& gpuPrograms,
            GFXGraphicsPipelineStateParams stateParams,
            const array_list<GFXDescriptorSetLayout_sp>& descriptorSetLayouts,
            const GFXRenderTargetDesc& renderTargetDesc,
            const GFXGraphicsPipelineState& gpInfo) = 0;

        virtual void GCollect() = 0;

    };
}