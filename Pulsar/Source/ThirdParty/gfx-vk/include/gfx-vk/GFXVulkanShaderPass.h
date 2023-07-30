#pragma once
#include <gfx/GFXShaderPass.h>
#include <gfx/GFXDescriptorSet.h>
#include "VulkanInclude.h"
#include "GFXVulkanRenderPass.h"
#include "GFXVulkanGpuProgram.h"

namespace gfx
{
    class GFXVulkanApplication;

    struct VkShaderStateData
    {
        VkPipelineRasterizationStateCreateInfo RasterizationState;
        VkPipelineColorBlendStateCreateInfo ColorBlendState;
        VkPipelineDepthStencilStateCreateInfo DepthStencilState;
    };
    
    class GFXVulkanShaderPass : public GFXShaderPass
    {
    public:
        GFXVulkanShaderPass(
            GFXVulkanApplication* app,
            const GFXShaderPassConfig& config,
            const std::shared_ptr<GFXVulkanGpuProgram>& gpuProgram,
            const std::shared_ptr<GFXDescriptorSetLayout>& descriptorSetLayout,
            const std::shared_ptr<GFXVertexLayoutDescription>& vertexLayout);

        GFXVulkanShaderPass(const GFXVulkanShaderPass&) = delete;

        virtual ~GFXVulkanShaderPass() override;

    public:
        virtual GFXShaderPassConfig GetStateConfig() const override { return m_passConfig; }
        virtual std::shared_ptr<GFXDescriptorSetLayout> GetDescriptorSetLayout() const override { return m_descriptorSetLayout; }
        virtual std::shared_ptr<GFXVertexLayoutDescription> GetVertexLayout() const override { return m_vertexLayoutDescription; }

        const std::vector<VkPipelineShaderStageCreateInfo>& GetVkStages() const { return m_stages; }
        GFXVulkanApplication* GetApplication() const { return m_app; }
    protected:
        GFXShaderPassConfig m_passConfig;

        std::vector<VkPipelineShaderStageCreateInfo> m_stages;
        std::shared_ptr<GFXVulkanGpuProgram> m_gpuProgram;
        std::shared_ptr<GFXDescriptorSetLayout> m_descriptorSetLayout;
        std::shared_ptr<GFXVertexLayoutDescription> m_vertexLayoutDescription;

        GFXVulkanApplication* m_app;
    };
}