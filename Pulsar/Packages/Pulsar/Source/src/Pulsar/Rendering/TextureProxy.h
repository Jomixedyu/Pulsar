#pragma once

#include <Pulsar/Rendering/RenderProxy.h>

#include <gfx/GFXTexture.h>

#include <vector>

namespace pulsar::rendering
{
    class TextureProxy final : public RenderProxy
    {
    public:
        TextureProxy(int32_t width, int32_t height,
                     gfx::GFXTextureFormat format,
                     const gfx::GFXSamplerConfig& sampler,
                     std::vector<uint8_t> data);

        explicit TextureProxy(int32_t size, const gfx::GFXSamplerConfig& sampler);

        void OnCreateResource() override;
        void OnDestroyResource() override;

        bool IsCreated() const { return m_created; }
        gfx::GFXTexturePtr GetTexture() const { return m_texture; }

    private:
        void CreateResource();
        void DestroyResource();

        int32_t m_width = 1;
        int32_t m_height = 1;
        gfx::GFXTextureDataType m_dataType = gfx::GFXTextureDataType::Texture2D;
        gfx::GFXTextureFormat m_format{};
        gfx::GFXSamplerConfig m_sampler{};
        std::vector<uint8_t> m_data;

        gfx::GFXTexturePtr m_texture;
        bool m_created = false;
    };
}
