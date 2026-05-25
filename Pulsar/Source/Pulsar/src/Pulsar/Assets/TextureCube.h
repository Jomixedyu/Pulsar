#pragma once
#include "Texture.h"
#include <Pulsar/Rendering/RenderProxyTexture.h>

namespace pulsar
{
    class TextureCube : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::TextureCube, Texture);

    public:
        bool CreateGPUResource() override;
        void DestroyGPUResource() override;
        bool IsCreatedGPUResource() const override { return m_isCreatedGPUResource; }

        std::shared_ptr<gfx::GFXTexture> GetGFXTexture() const override;

    public:
        array_list<uint8_t> m_originData;

        SPtr<RenderProxyTextureCube> m_proxy;

        bool m_isCreatedGPUResource{};

        int32_t m_width{};
    };
}