#pragma once
#include "Texture.h"

namespace pulsar
{
    class RenderTextureCube : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RenderTextureCube, Texture);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute);
    public:
        RenderTextureCube();
        bool CreateGPUResource() override;
        void DestroyGPUResource() override;
        bool IsCreatedGPUResource() const override;
        int32_t GetWidth() const override;
        int32_t GetHeight() const override;

        void PostEditChange(FieldInfo* info) override;

    protected:
        void SetWidth(int32_t width);

    protected:
        CORELIB_REFL_DECL_FIELD(m_width);
        int32_t m_width;

        gfx::GFXTexture_sp m_texture;
        bool m_isCreated = false;
    };
} // namespace pulsar