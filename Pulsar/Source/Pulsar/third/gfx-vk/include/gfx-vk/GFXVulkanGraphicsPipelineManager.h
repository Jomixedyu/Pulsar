#pragma once
#include <gfx/GFXGraphicsPipelineManager.h>
#include <unordered_map>

namespace gfx
{
    class GFXVulkanApplication;
    class GFXVulkanGraphicsPipelineManager : public GFXGraphicsPipelineManager
    {
    public:
        GFXVulkanGraphicsPipelineManager(GFXVulkanApplication* app);
        virtual ~GFXVulkanGraphicsPipelineManager() override;
    public:
        virtual std::shared_ptr<GFXGraphicsPipeline> GetGraphicsPipeline(
            const std::shared_ptr<GFXShaderPass>& shaderPass,
            const array_list<GFXDescriptorSetLayout_sp>& descriptorSetLayouts,
            const std::shared_ptr<GFXRenderPassLayout>& renderPass) override;

        virtual void GCollect() override;

    protected:
        GFXVulkanApplication* m_app;
        std::unordered_map<intptr_t, std::shared_ptr<GFXGraphicsPipeline>> m_caches;
    };
}