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
            const GFXVulkanGpuProgram_sp& gpuProgram);

        GFXVulkanShaderPass(const GFXVulkanShaderPass&) = delete;

        virtual ~GFXVulkanShaderPass() override;

    public:
        virtual GFXShaderPassConfig                         GetStateConfig() const override { return m_passConfig; }
        //virtual array_list<GFXDescriptorSetLayout_sp>       GetDescriptorSetLayout() const override { return m_descriptorSetLayout; }
        //array_list<VkDescriptorSetLayout>                   GetVkDescriptorSetLayout() const;
        const array_list<VkPipelineShaderStageCreateInfo>&  GetVkStages() const { return m_stages; }

        GFXVulkanApplication* GetApplication() const { return m_app; }

    protected:
        GFXShaderPassConfig                         m_passConfig;

        array_list<VkPipelineShaderStageCreateInfo> m_stages;
        GFXVulkanGpuProgram_sp                      m_gpuProgram;
        array_list<GFXDescriptorSetLayout_sp>       m_descriptorSetLayout;

        GFXVulkanApplication* m_app;
    };
}