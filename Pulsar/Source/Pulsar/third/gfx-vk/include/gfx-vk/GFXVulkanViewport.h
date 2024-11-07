#pragma once
#include "VulkanInclude.h"
#include <vector>
#include <memory>
#include "GFXVulkanCommandBuffer.h"

#include "GFXVulkanFrameBufferObject.h"
#include "GFXVulkanQueue.h"
#include "GFXVulkanRenderPass.h"
#include "GFXVulkanTexture.h"
#include <gfx/GFXViewport.h>
#include <gfx/GFXSurface.h>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanViewport : public GFXViewport
    {

    public:

        GFXVulkanViewport(GFXVulkanApplication* app, GFXSurface* window);
        virtual ~GFXVulkanViewport() override;

        void InitSwapChain();
        void TermSwapChain();

        void ReInitSwapChain();
    public:
        virtual void SetSize(int width, int height) override;
        virtual void GetSize(int* width, int* height) const override;
    public:
        VkSwapchainKHR GetVkSwapChain() const { return m_swapChain; }
        VkImage GetVkSwapChainImage() const { return m_swapChainImages[m_imageIndex]; }
        VkImageView GetVkSwapChainImageView() const { return m_swapChainImageViews[m_imageIndex]; }
        VkExtent2D GetVkSwapChainExtent() const { return m_swapChainExtent; }
        VkFormat GetVkSwapChainImageFormat() const { return m_swapChainImageFormat; }
        GFXVulkanApplication* GetApplication() const { return m_app; }
        GFXVulkanQueue* GetQueue() const { return m_queues[m_currentFrame].get(); }
        VkResult AcquireNextImage(uint32_t* outIndex);
    public:
        virtual GFXFrameBufferObject* GetFrameBufferObject() override;
        GFXVulkanRenderPass* GetRenderPass() const { return m_renderPass.get(); }
    protected:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
        bool m_framebufferResized = false;

        VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;

        std::vector<std::unique_ptr<GFXVulkanFrameBufferObject>> m_framebuffer;

        std::vector< std::unique_ptr<GFXVulkanQueue> > m_queues;

        std::vector<std::unique_ptr<GFXVulkanTexture>> m_swapRenderTarget;

        std::unique_ptr<GFXVulkanTexture> m_depthRenderTarget = nullptr;

        std::shared_ptr<GFXVulkanRenderPass> m_renderPass;

        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;

        uint32_t m_imageIndex = 0;
    public:
        int32_t m_currentFrame = 0;

        GFXVulkanApplication* m_app;
        GFXSurface* m_window = nullptr;
    };
}