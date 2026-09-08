#pragma once
#include "Texture.h"

namespace pulsar::rendering
{
    class TextureProxy;
}


namespace pulsar
{
    class TextureCube : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::TextureCube, Texture);

    public:
        bool CreateGPUResource() override;
        void DestroyGPUResource() override;
        bool IsCreatedGPUResource() const override { return m_isCreatedGPUResource; }
        gfx::TextureHandle GetTextureHandle() const override;

    public:
        array_list<uint8_t> m_originData;

        std::shared_ptr<rendering::TextureProxy> m_proxy;

        bool m_isCreatedGPUResource{};

        int32_t m_width{};
    };
}
