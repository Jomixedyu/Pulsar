#pragma once
#include <gfx/GFXShaderPass.h>
#include <gfx/GFXDescriptorSet.h>
#include "VulkanInclude.h"
#include "GFXVulkanRenderPass.h"
#include "GFXVulkanGpuProgram.h"

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanShaderPass : public GFXShaderPass
    {
    public:
        GFXVulkanShaderPass(
            GFXVulkanApplication* app,
            const GFXShaderPassConfig& config,
            const GFXVulkanGpuProgram_sp& gpuProgram,
            const GFXDescriptorSetLayout_sp& descriptorSetLayout,
            const array_list<GFXVertexLayoutDescription_sp>& vertexLayout);

        GFXVulkanShaderPass(const GFXVulkanShaderPass&) = delete;

        virtual ~GFXVulkanShaderPass() override;

    public:
        virtual GFXShaderPassConfig                         GetStateConfig() const override { return m_passConfig; }
        virtual GFXDescriptorSetLayout_sp                   GetDescriptorSetLayout() const override { return m_descriptorSetLayout; }
        virtual array_list<GFXVertexLayoutDescription_sp>   GetVertexLayout() const override { return m_vertexLayoutDescription; }
        const array_list<VkPipelineShaderStageCreateInfo>&  GetVkStages() const { return m_stages; }

        GFXVulkanApplication* GetApplication() const { return m_app; }

    protected:
        GFXShaderPassConfig                         m_passConfig;

        array_list<VkPipelineShaderStageCreateInfo> m_stages;
        GFXVulkanGpuProgram_sp                      m_gpuProgram;
        GFXDescriptorSetLayout_sp                   m_descriptorSetLayout;
        array_list<GFXVertexLayoutDescription_sp>   m_vertexLayoutDescription;

        GFXVulkanApplication* m_app;
    };
}