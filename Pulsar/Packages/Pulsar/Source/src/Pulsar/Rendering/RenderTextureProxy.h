#pragma once

#include <Pulsar/Rendering/RenderProxy.h>
#include <Pulsar/Assets/RenderTextureFormats.h>

#include <gfx/GFXFrameBufferObject.h>
#include <gfx/GFXTexture.h>

#include <memory>

#include <vector>

namespace pulsar::rendering
{
    class RenderTextureProxy final : public RenderProxy
    {
    public:
        RenderTextureProxy(int32_t width, int32_t height, uint32_t sampleCount,
                           RenderTextureColorFormat colorFormat,
                           RenderTextureDepthFormat depthFormat);

        void OnCreateResource() override;
        void OnDestroyResource() override;

        bool IsCreated() const { return m_created; }

        gfx::GFXTexturePtr GetColorTexture() const { return m_colorTexture; }
        gfx::GFXTexturePtr GetDepthTexture() const { return m_depthTexture; }

        gfx::GFXTexture2DViewPtr GetColorTextureView() const;
        gfx::GFXTexture2DViewPtr GetDepthRenderTarget() const;
        gfx::GFXFrameBufferObjectPtr GetFrameBufferObject() const;
        array_list<gfx::GFXTexturePtr> GetFramebufferAttachments() const;

    private:
        void CreateResources();
        void DestroyResources();

        int32_t m_width = 1;
        int32_t m_height = 1;
        uint32_t m_sampleCount = 1;
        RenderTextureColorFormat m_colorFormat = RenderTextureColorFormat::RGBA8_UNorm;
        RenderTextureDepthFormat m_depthFormat = RenderTextureDepthFormat::None;

        gfx::GFXTexturePtr m_colorTexture;
        gfx::GFXTexturePtr m_depthTexture;
        gfx::GFXFrameBufferObjectPtr m_framebuffer;
        bool m_created = false;
    };
}
