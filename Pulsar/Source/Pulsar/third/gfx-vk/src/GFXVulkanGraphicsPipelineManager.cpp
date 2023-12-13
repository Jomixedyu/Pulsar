#include "GFXVulkanGraphicsPipelineManager.h"
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
        const std::shared_ptr<GFXShaderPass>& shaderPass,
        const array_list<GFXDescriptorSetLayout_sp>& descriptorSetLayouts,
        const std::shared_ptr<GFXRenderPassLayout>& renderPass,
        const GFXGraphicsPipelineState& gpInfo)
    {
        auto hash = (HashPointer2(shaderPass.get(), renderPass.get()) ^ gpInfo.GetHashCode()) * 16777619;

        auto v = m_caches.find(hash);
        if (v != m_caches.end())
        {
            return v->second;
        }
        auto pipeline = new GFXVulkanGraphicsPipeline(m_app, shaderPass, descriptorSetLayouts, *renderPass.get(), gpInfo);
        auto gpipeline = gfxmksptr(pipeline);

        m_caches.insert({ hash, gpipeline });
        return gpipeline;
    }

    void GFXVulkanGraphicsPipelineManager::GCollect()
    {

    }
}
