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
    std::shared_ptr<GFXGraphicsPipeline> GFXVulkanGraphicsPipelineManager::GetGraphicsPipeline(
        const std::shared_ptr<GFXShaderPass>& shaderPass,
        const std::shared_ptr<GFXRenderPassLayout>& renderPass)
    {
        auto hash = (uint64_t)shaderPass.get() ^ (uint64_t)renderPass.get();
        
        auto v = m_caches.find(hash);
        if (v != m_caches.end())
        {
            return v->second;
        }

        auto gpipeline = std::shared_ptr<GFXGraphicsPipeline>(new GFXVulkanGraphicsPipeline(m_app, shaderPass, *renderPass.get()));
        m_caches.insert({ hash, gpipeline });
        return gpipeline;
    }

    void GFXVulkanGraphicsPipelineManager::GCollect()
    {

    }
}
