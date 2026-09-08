#include <Pulsar/Rendering/TextureProxy.h>

#include <Pulsar/Application.h>
#include <gfx/GFXResourceManager.h>

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
        if (auto gfxApp = Application::GetGfxApp())
            m_handle = gfxApp->GetResourceManager()->AllocHandle<gfx::TextureHandle>();
    }

    TextureProxy::TextureProxy(int32_t size, const gfx::GFXSamplerConfig& sampler)
        : m_width(size)
        , m_height(size)
        , m_dataType(gfx::GFXTextureDataType::TextureCube)
        , m_sampler(sampler)
    {
        if (auto gfxApp = Application::GetGfxApp())
            m_handle = gfxApp->GetResourceManager()->AllocHandle<gfx::TextureHandle>();
    }

    void TextureProxy::OnCreateResource()
    {
        if (m_created)
            return;

        CreateResource();
    }

    void TextureProxy::OnDestroyResource()
    {
        DestroyResource();
    }

    void TextureProxy::CreateResource()
    {
        if (m_created || !m_handle.IsValid())
            return;

        auto gfxApp = Application::GetGfxApp();
        if (!gfxApp)
            return;

        if (m_dataType == gfx::GFXTextureDataType::TextureCube)
        {
            gfxApp->GetResourceManager()->CreateTextureCube(m_handle, m_width);
        }
        else
        {
            gfx::GFXTextureCreateDesc desc{};
            desc.ImageData = m_data.data();
            desc.DataLength = m_data.size();
            desc.Width = m_width;
            desc.Height = m_height;
            desc.Format = m_format;
            desc.SamplerCfg = m_sampler;

            gfxApp->GetResourceManager()->CreateTexture2D(m_handle, desc);
        }

        m_created = true;
    }

    void TextureProxy::DestroyResource()
    {
        if (!m_created || !m_handle.IsValid())
            return;

        if (auto gfxApp = Application::GetGfxApp())
            gfxApp->GetResourceManager()->Destroy(m_handle);

        m_handle = {};
        m_created = false;
    }
}
