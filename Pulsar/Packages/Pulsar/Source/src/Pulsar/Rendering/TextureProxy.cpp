#include <Pulsar/Rendering/TextureProxy.h>

#include <Pulsar/Application.h>
#include <gfx/GFXResourceRegistry.h>

namespace pulsar::rendering
{
    void TextureProxy::Init(int32_t width, int32_t height,
                            gfx::GFXTextureFormat format,
                            const gfx::GFXSamplerConfig& sampler,
                            std::vector<uint8_t> data)
    {
        Release();

        auto gfxApp = Application::GetGfxApp();
        auto* registry = gfxApp ? gfxApp->GetResourceRegistry() : nullptr;
        if (!registry)
            return;

        gfx::GFXTextureCreateDesc desc{};
        desc.Width = width;
        desc.Height = height;
        desc.DataType = gfx::GFXTextureDataType::Texture2D;
        desc.Format = format;
        desc.SamplerCfg = sampler;

        m_texture = registry->CreateTexture2D(desc);
        if (!data.empty())
            m_texture->UpdateTexture({ data.data(), data.size() });

        m_nativeViewHandle.store(m_texture ? m_texture->GetNativeViewHandle() : 0, std::memory_order_release);
        m_initialized = true;
    }

    void TextureProxy::InitCube(int32_t size, const gfx::GFXSamplerConfig& sampler)
    {
        Release();

        auto gfxApp = Application::GetGfxApp();
        auto* registry = gfxApp ? gfxApp->GetResourceRegistry() : nullptr;
        if (!registry)
            return;

        m_texture = registry->CreateTextureCube(size);

        m_nativeViewHandle.store(m_texture ? m_texture->GetNativeViewHandle() : 0, std::memory_order_release);
        m_initialized = true;
    }

    void TextureProxy::Release()
    {
        // Member reset only; the registry defers the actual GPU destruction.
        // Safe from any thread (also runs from the destructor).
        m_nativeViewHandle.store(0, std::memory_order_release);
        m_texture.reset();
        m_initialized = false;
    }
}