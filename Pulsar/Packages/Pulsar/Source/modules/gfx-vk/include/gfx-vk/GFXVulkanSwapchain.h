#pragma once
#include "VulkanInclude.h"
#include <vector>
#include <memory>
#include "GFXVulkanCommandBuffer.h"

#include "GFXVulkanFrameBufferObject.h"
#include "GFXVulkanTexture.h"
#include <gfx/GFXSurface.h>
#include <gfx/GFXSwapchain.h>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanSwapchain : public GFXSwapchain
    {

    public:

        GFXVulkanSwapchain(GFXVulkanApplication* app, GFXSurface* window);
        virtual ~GFXVulkanSwapchain() override;

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
        // Sync primitives for the current frame slot (indexed by frame-in-flight).
        VkSemaphore GetImageAvailableSemaphore() const;
        VkResult AcquireNextImage(uint32_t* outIndex);
        VkSemaphore GetRenderFinishedSemaphore(uint32_t imageIndex) const { return m_renderFinishedSemaphores[imageIndex]; }
    public:
        virtual GFXFrameBufferObject* GetFrameBufferObject() override;
    protected:
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        // Indexed by swapchain image (present completion is tied to the image,
        // not the frame slot).
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        bool m_framebufferResized = false;

        VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;

        std::vector<std::unique_ptr<GFXVulkanFrameBufferObject>> m_framebuffer;

        std::vector<std::unique_ptr<GFXVulkanTexture>> m_swapRenderTarget;

        std::unique_ptr<GFXVulkanTexture> m_depthRenderTarget = nullptr;

        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;

        uint32_t m_imageIndex = 0;
    public:
        GFXVulkanApplication* m_app;
        GFXSurface* m_window = nullptr;
    };
}
