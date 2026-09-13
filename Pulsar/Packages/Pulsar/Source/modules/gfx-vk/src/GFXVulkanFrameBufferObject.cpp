#include "GFXVulkanFrameBufferObject.h"
#include "GFXVulkanApplication.h"
#include "GFXVulkanResourceRegistry.h"
#include <cassert>

namespace gfx
{
    GFXVulkanFrameBufferObject::GFXVulkanFrameBufferObject(
        GFXResourceRegistry* registry,
        const std::vector<GFXTexture2DViewPtr>& renderTargets)
        : GFXFrameBufferObject(registry), m_renderTargets(renderTargets)
    {
        assert(renderTargets.size() != 0);

        auto& first = renderTargets[0];
        m_width = first->GetWidth();
        m_height = first->GetHeight();

        m_id = GetResourceId();

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
        if (!m_renderTargets.empty())
        {
            m_renderTargetDesc.SampleCount = m_renderTargets[0]->GetTexture()->GetSampleCount();
        }
    }

    GFXVulkanFrameBufferObject::~GFXVulkanFrameBufferObject()
    {
    }
}
