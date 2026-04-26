#pragma once
#include "VulkanInclude.h"
#include "gfx/GFXDescriptorSet.h"
#include "gfx/GFXGpuProgram.h"
#include <gfx/GFXGraphicsPipeline.h>
#include <gfx/GFXRenderPass.h>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanGraphicsPipeline : public GFXGraphicsPipeline
    {
        using base = GFXGraphicsPipeline;
    public:
        GFXVulkanGraphicsPipeline(
            GFXVulkanApplication* app,
            const array_list<GFXGpuProgram_sp>& gpuPrograms,
            GFXGraphicsPipelineStateParams stateParams,
            const array_list<GFXDescriptorSetLayout_sp>& descriptorSetLayouts,
            const GFXRenderTargetDesc& renderTargetDesc,
            const GFXGraphicsPipelineState& gpInfo);

        virtual ~GFXVulkanGraphicsPipeline() override;

    public:
        const VkPipelineLayout& GetVkPipelineLayout() const { return m_pipelineLayout; }
        const VkPipeline& GetVkPipeline() const { return m_pipeline; }
        GFXVulkanApplication* GetApplication() const { return m_app; }
    protected:

        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_pipeline = VK_NULL_HANDLE;
        GFXVulkanApplication* m_app;
    };
    static VkPolygonMode _GetVkPolyMode(GFXPrimitiveTopology topology);
}