#include "GFXVulkanRenderer.h"
#include "GFXVulkanApplication.h"
#include "GFXVulkanBufferUploadQueue.h"
#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanQueue.h"
#include "GFXVulkanFrameBufferObject.h"
#include <SDL_vulkan.h>
#include <array>

namespace gfx
{

    GFXVulkanRenderer::GFXVulkanRenderer(GFXVulkanApplication* app)
        : m_app(app)
    {
    }

    bool GFXVulkanRenderer::Render(float deltaTime)
    {
        const auto viewport = m_app->GetVulkanViewport();

        if (SDL_GetWindowFlags((SDL_Window*)m_app->GetWindow()->GetUserPoint()) & SDL_WINDOW_MINIMIZED)
        {
            return false;
        }

        vkWaitForFences(m_app->GetVkDevice(), 1, &viewport->GetQueue()->GetVkFence(), VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        auto result = m_app->GetVulkanViewport()->AcquireNextImage(&imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_app->GetVulkanViewport()->ReInitSwapChain();
            return false;
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
        if (!renderTargets)
        {
            return false;
        }

        m_app->GetRenderPipeline()->OnRecord(&renderContext, renderTargets);

        // Frame-head transfer: flush all staging->device copies queued during this frame (both the
        // resource-update drain and lazy variant creation inside OnRecord) into a command buffer
        // prepended to the front, so it executes (and its barrier makes the writes visible) before
        // the render command buffers that read the uploaded buffers.
        if (auto* uploadQueue = m_app->GetBufferUploadQueue(); uploadQueue->HasPending())
        {
            auto& transferCmd = renderContext.PrependCommandBuffer();
            transferCmd.Begin();
            uploadQueue->FlushInto(transferCmd.GetVkCommandBuffer());
            transferCmd.End();
        }

        renderContext.Submit();

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        VkSwapchainKHR swapChains[] = {m_app->GetVulkanViewport()->GetVkSwapChain()};
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

        // The staging ring recycles its slot lazily on the next frame (keyed on the gfx frame
        // counter), so there is nothing to advance here; this frame's uploads were already
        // flushed above. Returning true tells the caller to advance the frame counter.
        return true;
    }
    void GFXVulkanRenderer::WaitExecuteRender(const std::function<void(GFXRenderContext*)>& func)
    {
        const auto viewport = m_app->GetVulkanViewport();

    }

    GFXVulkanRenderer::~GFXVulkanRenderer()
    {

    }
}