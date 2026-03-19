#include "GFXVulkanGraphicsPipelineManager.h"

#include "GFXVulkanGpuProgram.h"
#include "GFXVulkanGraphicsPipeline.h"

namespace gfx
{
    GFXVulkanGraphicsPipelineManager::~GFXVulkanGraphicsPipelineManager()
    {

    }
    GFXVulkanGraphicsPipelineManager::GFXVulkanGraphicsPipelineManager(GFXVulkanApplication* app)
        : m_app(app)
    {

    }


    inline intptr_t HashPointer2(void* p, void* q)
    {
        constexpr intptr_t HashS0 = 2166136261;
        constexpr intptr_t HashS1 = 16777619;
        return ((HashS0 * HashS1) ^ (intptr_t)p) * HashS1 ^ (intptr_t)q;
    }

    std::shared_ptr<GFXGraphicsPipeline> GFXVulkanGraphicsPipelineManager::GetGraphicsPipeline(
        const array_list<GFXGpuProgram_sp>& gpuPrograms,
        GFXGraphicsPipelineStateParams stateParams,
        const array_list<GFXDescriptorSetLayout_sp>& descriptorSetLayouts,
        const GFXRenderTargetDesc& renderTargetDesc,
        const GFXGraphicsPipelineState& gpInfo)
    {
        constexpr intptr_t HashS0 = 2166136261;
        constexpr intptr_t HashS1 = 16777619;

        uint64_t hash = stateParams.GetHashCode();
        for (auto& gpuProgram : gpuPrograms)
        {
            hash = hash * HashS1 ^ (uintptr_t)gpuProgram.get();
        }
        hash = hash * HashS1 ^ renderTargetDesc.GetHashCode();

        auto v = m_caches.find(hash);
        if (v != m_caches.end())
        {
            return v->second;
        }
        auto pipeline = new GFXVulkanGraphicsPipeline(m_app, gpuPrograms, stateParams, descriptorSetLayouts, renderTargetDesc, gpInfo);
        auto gpipeline = gfxmksptr(pipeline);

        m_caches.insert({ hash, gpipeline });
        return gpipeline;
    }

    void GFXVulkanGraphicsPipelineManager::GCollect()
    {

    }
}
