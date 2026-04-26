#pragma once
#include "Texture.h"


namespace pulsar
{
    class TextureCube : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::TextureCube, Texture);

    public:
        bool CreateGPUResource() override;
        void DestroyGPUResource() override;
        bool IsCreatedGPUResource() const override { return m_isCreatedGPUResource; }

    public:
        array_list<uint8_t> m_originData;

        gfx::GFXTexture_sp m_cube;

        bool m_isCreatedGPUResource{};

        int32_t m_width{};
    };
}