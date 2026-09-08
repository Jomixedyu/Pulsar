#pragma once

#include <Pulsar/Rendering/RenderProxy.h>
#include <Pulsar/Assets/RenderTextureFormats.h>

#include <gfx/GFXFrameBufferObject.h>
#include <gfx/GFXHandle.h>
#include <gfx/GFXTexture.h>

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

        gfx::TextureHandle GetTextureHandle() const;
        gfx::TextureHandle GetDepthTextureHandle() const;

        gfx::GFXTexture2DView_sp GetColorTextureView() const;
        gfx::GFXTexture2DView_sp GetDepthRenderTarget() const;
        gfx::GFXFrameBufferObject_sp GetFrameBufferObject() const;
        array_list<gfx::GFXTexture_sp> GetFramebufferAttachments() const;

    private:
        void CreateResources();
        void DestroyResources();

        int32_t m_width = 1;
        int32_t m_height = 1;
        uint32_t m_sampleCount = 1;
        RenderTextureColorFormat m_colorFormat = RenderTextureColorFormat::RGBA8_UNorm;
        RenderTextureDepthFormat m_depthFormat = RenderTextureDepthFormat::None;

        gfx::TextureHandle m_colorHandle{};
        gfx::TextureHandle m_depthHandle{};
        gfx::FrameBufferObjectHandle m_framebufferHandle{};
        gfx::GFXFrameBufferObject_sp m_framebuffer;
        bool m_created = false;
    };
}
