#include "GFXVulkanFrameBufferObject.h"
#include "GFXVulkanApplication.h"
#include <cassert>

namespace gfx
{
    static int32_t idNext = 0;

    GFXVulkanFrameBufferObject::GFXVulkanFrameBufferObject(
        GFXVulkanApplication* app,
        const std::vector<GFXTexture2DView_sp>& renderTargets)
        : m_app(app), m_renderTargets(renderTargets)
    {
        assert(renderTargets.size() != 0);

        auto& first = renderTargets[0];
        m_width = first->GetWidth();
        m_height = first->GetHeight();

        m_id = ++idNext;

        // Deduce GFXRenderTargetDesc from render targets
        for (auto& rt : m_renderTargets)
        {
            auto targetType = rt->GetTargetType();
            if (targetType == GFXTextureTargetType::ColorTarget)
            {
                m_renderTargetDesc.ColorFormats.push_back(rt->GetFormat());
            }
            else if (targetType == GFXTextureTargetType::DepthStencilTarget ||
                     targetType == GFXTextureTargetType::DepthTarget)
            {
                m_renderTargetDesc.DepthStencilFormat = rt->GetFormat();
                m_renderTargetDesc.HasDepthStencil = true;
            }
        }
    }

    GFXVulkanFrameBufferObject::~GFXVulkanFrameBufferObject()
    {
    }
}