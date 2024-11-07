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
        void Render(float deltaTime);

        void WaitExecuteRender(const std::function<void(GFXRenderContext*)>& func) override;

    protected:
        GFXVulkanApplication* m_app;

        bool m_framebufferResized = false;
        uint32_t m_currentFrame = 0;
        const int MAX_FRAMES_IN_FLIGHT = 2;

    };
}