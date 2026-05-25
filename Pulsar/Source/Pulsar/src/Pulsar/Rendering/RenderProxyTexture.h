#pragma once

#include <gfx/GFXTexture.h>
#include <gfx/GFXFrameBufferObject.h>
#include <memory>
#include <vector>

namespace pulsar
{
    class Texture2D;
    class TextureCube;
    class RenderTexture;
    class RenderTextureCube;

    class RenderProxyTexture2D
    {
    public:
        RenderProxyTexture2D() = default;
        explicit RenderProxyTexture2D(Texture2D* source);

        void SetSource(Texture2D* source);
        Texture2D* GetSource() const { return m_source; }

        void InitRHI();
        void ReleaseRHI();

        gfx::GFXTexture* GetGFXTexture() const { return m_tex.get(); }
        bool IsInitialized() const { return m_tex.get() != nullptr; }

    private:
        Texture2D* m_source = nullptr;
        std::shared_ptr<gfx::GFXTexture> m_tex;
    };

    class RenderProxyTextureCube
    {
    public:
        RenderProxyTextureCube() = default;
        explicit RenderProxyTextureCube(TextureCube* source);

        void SetSource(TextureCube* source);
        TextureCube* GetSource() const { return m_source; }

        void InitRHI();
        void ReleaseRHI();

        gfx::GFXTexture* GetGFXTexture() const { return m_tex.get(); }
        bool IsInitialized() const { return m_tex.get() != nullptr; }

    private:
        TextureCube* m_source = nullptr;
        std::shared_ptr<gfx::GFXTexture> m_tex;
    };

    class RenderProxyRenderTexture
    {
    public:
        RenderProxyRenderTexture() = default;
        explicit RenderProxyRenderTexture(RenderTexture* source);

        void SetSource(RenderTexture* source);
        RenderTexture* GetSource() const { return m_source; }

        void InitRHI();
        void ReleaseRHI();

        const std::vector<gfx::GFXTexture_sp>& GetRenderTargets() const { return m_renderTargets; }
        gfx::GFXFrameBufferObject_sp GetFrameBufferObject() const { return m_framebuffer; }
        gfx::GFXTexture_sp GetRenderTarget0() const { return m_renderTargets.empty() ? gfx::GFXTexture_sp{} : m_renderTargets[0]; }
        bool IsInitialized() const { return m_framebuffer.get() != nullptr; }

    private:
        RenderTexture* m_source = nullptr;
        std::vector<gfx::GFXTexture_sp> m_renderTargets;
        gfx::GFXFrameBufferObject_sp m_framebuffer;
    };

    class RenderProxyRenderTextureCube
    {
    public:
        RenderProxyRenderTextureCube() = default;
        explicit RenderProxyRenderTextureCube(RenderTextureCube* source);

        void SetSource(RenderTextureCube* source);
        RenderTextureCube* GetSource() const { return m_source; }

        void InitRHI();
        void ReleaseRHI();

        gfx::GFXTexture* GetGFXTexture() const { return m_tex.get(); }
        bool IsInitialized() const { return m_tex.get() != nullptr; }

    private:
        RenderTextureCube* m_source = nullptr;
        std::shared_ptr<gfx::GFXTexture> m_tex;
    };

} // namespace pulsar
