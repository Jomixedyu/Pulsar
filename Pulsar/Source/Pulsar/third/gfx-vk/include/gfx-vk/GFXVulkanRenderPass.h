#pragma once
#include <gfx/GFXRenderPass.h>
#include "VulkanInclude.h"
#include "GFXVulkanRenderTarget.h"

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanRenderPass : public GFXRenderPassLayout
    {

    public:
        //GFXVulkanRenderPass(GFXVulkanApplication* app, VkFormat colorFormat);
        GFXVulkanRenderPass(GFXVulkanApplication* app, const std::vector<GFXVulkanRenderTarget*>& createFromlayout);
        GFXVulkanRenderPass(const GFXVulkanRenderPass&) = delete;

        virtual ~GFXVulkanRenderPass() override;
    public:
        const VkRenderPass& GetVkRenderPass() const { return m_renderPass; }
        size_t GetColorAttachmentCount() const { return m_colorAttachmentCount; }
    protected:
        GFXVulkanApplication* m_app;
        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        size_t m_colorAttachmentCount{};
    };
}