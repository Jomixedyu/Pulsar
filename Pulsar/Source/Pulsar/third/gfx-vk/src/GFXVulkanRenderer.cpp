#include "GFXVulkanRenderer.h"
#include "GFXVulkanApplication.h"
#include "GFXVulkanRenderPass.h"
#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanQueue.h"
#include "GFXVulkanFrameBufferObject.h"
#include <array>

namespace gfx
{

    GFXVulkanRenderer::GFXVulkanRenderer(GFXVulkanApplication* app)
        : m_app(app)
    {
    }

    void GFXVulkanRenderer::Render(float deltaTime)
    {
        const auto viewport = m_app->GetVulkanViewport();

        vkWaitForFences(m_app->GetVkDevice(), 1, &viewport->GetQueue()->GetVkFence(), VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        auto result = m_app->GetVulkanViewport()->AcquireNextImage(&imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_app->GetVulkanViewport()->ReInitSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(m_app->GetVkDevice(), 1, &viewport->GetQueue()->GetVkFence());

        GFXVulkanRenderContext renderContext(m_app);

        renderContext.SetQueue(viewport->GetQueue());
        renderContext.DeltaTime = deltaTime;

        const auto renderTargets = viewport->GetFrameBufferObject();
        m_app->GetRenderPipeline()->OnRender(&renderContext, renderTargets);
        renderContext.Submit();

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        VkSwapchainKHR swapChains[] = { m_app->GetVulkanViewport()->GetVkSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pWaitSemaphores = &viewport->GetQueue()->GetVkSignalSemaphore();
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(m_app->GetVkPresentQueue(), &presentInfo);
        vkQueueWaitIdle(m_app->GetVkPresentQueue());

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
        {
            m_framebufferResized = false;
            m_app->GetVulkanViewport()->ReInitSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

    }

    GFXVulkanRenderer::~GFXVulkanRenderer()
    {

    }
}