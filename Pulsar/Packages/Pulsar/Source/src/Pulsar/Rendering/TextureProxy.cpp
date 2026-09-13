#include <Pulsar/Rendering/TextureProxy.h>

#include <Pulsar/Application.h>
#include <gfx/GFXResourceRegistry.h>

namespace pulsar::rendering
{
    TextureProxy::TextureProxy(int32_t width, int32_t height,
                               gfx::GFXTextureFormat format,
                               const gfx::GFXSamplerConfig& sampler,
                               std::vector<uint8_t> data)
        : m_width(width)
        , m_height(height)
        , m_dataType(gfx::GFXTextureDataType::Texture2D)
        , m_format(format)
        , m_sampler(sampler)
        , m_data(std::move(data))
    {
    }

    TextureProxy::TextureProxy(int32_t size, const gfx::GFXSamplerConfig& sampler)
        : m_width(size)
        , m_height(size)
        , m_dataType(gfx::GFXTextureDataType::TextureCube)
        , m_sampler(sampler)
    {
    }

    void TextureProxy::OnCreateResource()
    {
        if (m_created)
            return;

        auto gfxApp = Application::GetGfxApp();
        auto* registry = gfxApp ? gfxApp->GetResourceRegistry() : nullptr;
        if (!registry)
            return;

        if (m_dataType == gfx::GFXTextureDataType::TextureCube)
        {
            m_texture = registry->CreateTextureCube(m_width);
        }
        else
        {
            gfx::GFXTextureCreateDesc desc{};
            desc.ImageData = m_data.data();
            desc.DataLength = m_data.size();
            desc.Width = m_width;
            desc.Height = m_height;
            desc.DataType = m_dataType;
            desc.Format = m_format;
            desc.SamplerCfg = m_sampler;

            m_texture = registry->CreateTexture2D(desc);
        }

        m_created = true;
    }

    void TextureProxy::OnDestroyResource()
    {
        if (!m_created)
            return;

        m_texture.reset();
        m_created = false;
    }
}
