#include "GFXVulkanViewport.h"
#include "GFXVulkanViewport.h"
#include "GFXVulkanApplication.h"
#include "PhysicalDeviceHelper.h"
#include "BufferHelper.h"
#include <array>


#ifdef max
#undef max
#endif

namespace gfx
{
    namespace
    {
        struct SwapChainSupportDetails
        {
            VkSurfaceCapabilitiesKHR capabilities{};
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };


        static SwapChainSupportDetails _QuerySwapChainSupport(GFXVulkanApplication* app)
        {
            SwapChainSupportDetails details;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(app->GetVkPhysicalDevice(), app->GetVkSurface(), &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(app->GetVkPhysicalDevice(), app->GetVkSurface(), &formatCount, nullptr);

            if (formatCount != 0)
            {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(app->GetVkPhysicalDevice(), app->GetVkSurface(), &formatCount, details.formats.data());
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(app->GetVkPhysicalDevice(), app->GetVkSurface(), &presentModeCount, nullptr);

            if (presentModeCount != 0)
            {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(app->GetVkPhysicalDevice(), app->GetVkSurface(), &presentModeCount, details.presentModes.data());
            }

            return details;
        }
        static VkSurfaceFormatKHR _ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
        {
            for (const auto& availableFormat : availableFormats)
            {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    return availableFormat;
                }
            }

            return availableFormats[0];
        }
        static VkPresentModeKHR _ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
        {
            for (const auto& availablePresentMode : availablePresentModes)
            {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    return availablePresentMode;
                }
            }

            return VK_PRESENT_MODE_FIFO_KHR;
        }
        static VkExtent2D _ChooseSwapExtent(GFXVulkanApplication* app, const VkSurfaceCapabilitiesKHR& capabilities)
        {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            {
                return capabilities.currentExtent;
            }
            else
            {
                int width, height;
                glfwGetFramebufferSize(reinterpret_cast<GLFWwindow*>(app->GetWindowHandle()), &width, &height);

                VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };

                actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

                return actualExtent;
            }
        }
    }
    GFXVulkanViewport::GFXVulkanViewport(GFXVulkanApplication* app, GLFWwindow* window)
        : m_app(app), m_window(window)
    {
        this->InitSwapChain();
        //this->InitDepthTestBuffer();

        m_currentFrame = MAX_FRAMES_IN_FLIGHT - 1;

        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(app->GetVkDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(app->GetVkDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(app->GetVkDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_queues.push_back(std::unique_ptr<GFXVulkanQueue>{new GFXVulkanQueue(m_app, m_imageAvailableSemaphores[i], m_renderFinishedSemaphores[i], m_inFlightFences[i])});
        }
    }
    GFXVulkanViewport::~GFXVulkanViewport()
    {
        TermSwapChain();

        m_queues.clear();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(m_app->GetVkDevice(), m_imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(m_app->GetVkDevice(), m_renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(m_app->GetVkDevice(), m_inFlightFences[i], nullptr);
        }
        m_imageAvailableSemaphores.clear();
        m_renderFinishedSemaphores.clear();
        m_inFlightFences.clear();
        m_renderPass.reset();
    }


    void GFXVulkanViewport::InitSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = _QuerySwapChainSupport(m_app);
        VkSurfaceFormatKHR surfaceFormat = _ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = _ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = _ChooseSwapExtent(m_app, swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_app->GetVkSurface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;


        vk::QueueFamilyIndices indices = vk::PhysicalDeviceHelper::FindQueueFamilies(m_app->GetVkSurface(), m_app->GetVkPhysicalDevice());
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            //createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        }
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_app->GetVkDevice(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }

        //create swapchain images
        vkGetSwapchainImagesKHR(m_app->GetVkDevice(), m_swapChain, &imageCount, nullptr);
        m_swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_app->GetVkDevice(), m_swapChain, &imageCount, m_swapChainImages.data());

        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;

        GFXVulkanTexture2D* depthTexture = nullptr;
        //create depth image
        {
            VkFormat depthFormat = BufferHelper::FindDepthFormat(m_app);
            auto extent = m_swapChainExtent;

            VkImage depthImage;
            VkDeviceMemory depthMemory;

            BufferHelper::CreateImage(
                m_app, extent.width, extent.height, depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                depthImage, depthMemory);
            auto depthImageView = BufferHelper::CreateImageView(m_app, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
            BufferHelper::TransitionImageLayout(m_app, depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

            depthTexture = new GFXVulkanTexture2D(m_app, extent.width, extent.height, 1, depthFormat, depthImage, depthMemory, depthImageView, false, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, GFXSamplerConfig{});
            m_depthTex = std::unique_ptr<GFXVulkanTexture2D>{ depthTexture };
            m_depthRenderTarget = std::unique_ptr<GFXVulkanRenderTarget>(new GFXVulkanRenderTarget(m_depthTex.get(), GFXRenderTargetType::Depth));
        }


        //create swapchain image view
        m_swapChainImageViews.resize(m_swapChainImages.size());

        for (size_t i = 0; i < m_swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_app->GetVkDevice(), &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image views!");
            }

            auto tex2d = std::unique_ptr<GFXVulkanTexture2D>{ new GFXVulkanTexture2D(m_app, extent.width, extent.height, 4, m_swapChainImageFormat, m_swapChainImages[i], m_swapChainImageViews[i], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) };

            auto texRt = new GFXVulkanRenderTarget(tex2d.get(), GFXRenderTargetType::Color);

            m_swapRenderTarget.push_back(std::unique_ptr<GFXVulkanRenderTarget>(texRt));

            m_swapTex.push_back(std::move(tex2d));
        }

        m_renderPass = std::shared_ptr<GFXVulkanRenderPass>{ new GFXVulkanRenderPass(m_app, { m_swapRenderTarget[0].get(), m_depthRenderTarget.get() }) };

        for (size_t i = 0; i < m_swapChainImages.size(); i++)
        {
            m_framebuffer.push_back(std::unique_ptr<GFXVulkanFrameBufferObject>{ new GFXVulkanFrameBufferObject(m_app, { m_swapRenderTarget[i].get() , m_depthRenderTarget.get() }, m_renderPass) });
        }
    }

    void GFXVulkanViewport::TermSwapChain()
    {
        m_framebuffer.clear();
        m_swapRenderTarget.clear();
        m_swapTex.clear();

        m_depthRenderTarget.reset();
        m_depthTex.reset();

        for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
        {
            vkDestroyImageView(m_app->GetVkDevice(), m_swapChainImageViews[i], nullptr);
        }

        vkDestroySwapchainKHR(m_app->GetVkDevice(), m_swapChain, nullptr);
        m_renderPass.reset();
    }

    void GFXVulkanViewport::ReInitSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(m_window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_app->GetVkDevice());

        TermSwapChain();

        InitSwapChain();
    }

    void GFXVulkanViewport::SetSize(int width, int height)
    {
        glfwSetWindowSize(m_window, width, height);
    }

    void GFXVulkanViewport::GetSize(int* width, int* height) const
    {
        *width = m_swapChainExtent.width;
        *height = m_swapChainExtent.height;
    }
    VkResult GFXVulkanViewport::AcquireNextImage(uint32_t* outIndex)
    {
        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        auto result = vkAcquireNextImageKHR(m_app->GetVkDevice(), GetVkSwapChain(), UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_imageIndex);
        *outIndex = m_imageIndex;
        return result;
    }

    GFXFrameBufferObject* gfx::GFXVulkanViewport::GetFrameBufferObject()
    {
        return m_framebuffer[m_imageIndex].get();
    }
}