#pragma once
#include "VulkanInclude.h"
#include <vector>
#include <gfx/GFXCommandBuffer.h>
#include "GFXVulkanFrameBufferObject.h"
#include "GFXVulkanRenderContext.h"
#include <gfx/GFXRenderer.h>

namespace gfx
{
    class GFXVulkanApplication;

    class GFXVulkanRenderer : public GFXRenderer
    {
    public:
        GFXVulkanRenderer(GFXVulkanApplication* app);
        ~GFXVulkanRenderer();
    public:
        // Returns true if a frame was actually rendered+presented; false on an early-out
        // (minimized, swapchain out-of-date, no render target). The caller uses this to
        // advance the gfx frame counter only on successful frames.
        bool Render(float deltaTime);

        void WaitExecuteRender(const std::function<void(GFXRenderContext*)>& func) override;

    protected:
        GFXVulkanApplication* m_app;

        bool m_framebufferResized = false;

    };
}