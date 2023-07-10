#pragma once
#include <gfx/GFXShaderPass.h>
#include <gfx/GFXDescriptorSet.h>
#include <gfx-vk/VulkanInclude.h>
#include "GFXVulkanRenderPass.h"

namespace gfx
{
    class GFXVulkanApplication;
    class GFXVulkanShaderPass : public GFXShaderPass
    {
    public:
        GFXVulkanShaderPass(
            GFXVulkanApplication* app, 
            const GFXShaderPassConfig& config,
            std::shared_ptr<GFXVertexLayoutDescription> vertexLayout,
            std::shared_ptr<GFXShaderModule> shaderModule,
            const std::shared_ptr<GFXDescriptorSetLayout>& descSetLayout,
            GFXVulkanRenderPass* targetPass);
        virtual ~GFXVulkanShaderPass() override;

    public:
        const VkPipelineLayout& GetVkPipelineLayout() const { return m_pipelineLayout; }
        const VkPipeline& GetVkPipeline() const { return m_graphicsPipeline; }
    protected:
        GFXVulkanApplication* m_app;
        GFXVulkanRenderPass* m_targetRenderPass;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    };
}