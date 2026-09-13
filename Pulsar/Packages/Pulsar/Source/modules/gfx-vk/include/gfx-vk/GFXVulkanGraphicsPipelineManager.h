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
            const array_list<GFXGpuProgramPtr>& gpuPrograms,
            GFXGraphicsPipelineStateParams stateParams,
            const array_list<GFXDescriptorSetLayoutPtr>& descriptorSetLayouts,
            const GFXRenderTargetDesc& renderTargetDesc,
            const GFXGraphicsPipelineState& gpInfo) override;

        virtual void GCollect() override;

    protected:
        GFXVulkanApplication* m_app;
        std::unordered_map<intptr_t, std::shared_ptr<GFXGraphicsPipeline>> m_caches;
    };
}
