#pragma once

#include <Pulsar/Assets/RenderTextureFormats.h>

#include <gfx/GFXFrameBufferObject.h>
#include <gfx/GFXTexture.h>

#include <memory>
#include <vector>

namespace pulsar::rendering
{
    // Render-side mirror of a RenderTexture asset's render targets.
    // GPUResourceProxy convention: Init()/Release() run on the render thread
    // (via the render queue). Init is re-init safe -- calling it again rebuilds
    // the targets with new parameters.
    class RenderTextureProxy final
    {
    public:
        RenderTextureProxy() = default;
        ~RenderTextureProxy() { Release(); }

        void Init(int32_t width, int32_t height, uint32_t sampleCount,
                  RenderTextureColorFormat colorFormat,
                  RenderTextureDepthFormat depthFormat);
        void Release();

        bool IsInitialized() const { return m_initialized; }

        int32_t GetWidth() const { return m_colorTexture ? m_colorTexture->GetWidth() : 0; }
        int32_t GetHeight() const { return m_colorTexture ? m_colorTexture->GetHeight() : 0; }

        gfx::GFXTexturePtr GetColorTexture() const { return m_colorTexture; }
        gfx::GFXTexturePtr GetDepthTexture() const { return m_depthTexture; }

        gfx::GFXTexture2DViewPtr GetColorTextureView() const;
        gfx::GFXTexture2DViewPtr GetDepthRenderTarget() const;
        gfx::GFXFrameBufferObjectPtr GetFrameBufferObject() const;
        array_list<gfx::GFXTexturePtr> GetFramebufferAttachments() const;

    private:
        gfx::GFXTexturePtr m_colorTexture;
        gfx::GFXTexturePtr m_depthTexture;
        gfx::GFXFrameBufferObjectPtr m_framebuffer;
        bool m_initialized = false;
    };
}