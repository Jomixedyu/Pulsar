#pragma once
#include <gfx/GFXGraphicsPipeline.h>
#include <gfx/GFXShaderPass.h>
#include <gfx/GFXRenderPass.h>
#include "VulkanInclude.h"

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanGraphicsPipeline : public GFXGraphicsPipeline
    {
        using base = GFXGraphicsPipeline;
    public:
        GFXVulkanGraphicsPipeline(
            GFXVulkanApplication* app,
            const std::shared_ptr<GFXShaderPass>& shaderPass,
            const array_list<GFXDescriptorSetLayout_sp>& descriptorSetLayouts,
            const GFXRenderPassLayout& renderLayout);

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