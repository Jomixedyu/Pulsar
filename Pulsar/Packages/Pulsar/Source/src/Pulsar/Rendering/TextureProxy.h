#pragma once

#include <gfx/GFXTexture.h>

#include <atomic>
#include <vector>

namespace pulsar::rendering
{
    // Render-side mirror of a Texture asset's GPU texture.
    // GPUResourceProxy convention: Init()/Release() run on the render thread
    // (via the render queue). Init is re-init safe -- calling it again rebuilds
    // the texture with new parameters (shared holders see the swap
    // automatically). Destruction of the texture itself is deferred by the
    // registry.
    class TextureProxy final
    {
    public:
        TextureProxy() = default;
        ~TextureProxy() { Release(); }

        void Init(int32_t width, int32_t height,
                  gfx::GFXTextureFormat format,
                  const gfx::GFXSamplerConfig& sampler,
                  std::vector<uint8_t> data);
        void InitCube(int32_t size, const gfx::GFXSamplerConfig& sampler);
        void Release();

        bool IsInitialized() const { return m_initialized; }
        gfx::GFXTexturePtr GetTexture() const { return m_texture; }

        // The texture's backend view handle, safe to read from any thread
        // (0 until initialized).
        uint64_t GetNativeViewHandle() const { return m_nativeViewHandle.load(std::memory_order_acquire); }

    private:
        gfx::GFXTexturePtr m_texture;
        std::atomic<uint64_t> m_nativeViewHandle{ 0 };
        bool m_initialized = false;
    };
}