#pragma once
#include "CurveLinearColor.h"
#include "Texture.h"

namespace pulsar
{
    class CurveLinearColorAtlas : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CurveLinearColorAtlas, Texture);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute);
    public:
        CurveLinearColorAtlas();
        void Serialize(AssetSerializer* s) override;
        void Generate();
        bool CreateGPUResource() override;
        void DestroyGPUResource() override;
        bool IsCreatedGPUResource() const override { return m_isCreatedGpuResource; }
        int32_t GetWidth() const override;
        int32_t GetHeight() const override;
        std::shared_ptr<gfx::GFXTexture> GetGFXTexture() const override { return m_gfxTexture; }

        void PostEditChange(FieldInfo* info) override;
    protected:

        CORELIB_REFL_DECL_FIELD(m_width, new RangePropertyAttribute(4, 2048))
        int m_width;

        CORELIB_REFL_DECL_FIELD(m_height, new RangePropertyAttribute(4, 2048))
        int m_height;

        CORELIB_REFL_DECL_FIELD(m_colorCurveAssets, new ListItemAttribute(cltypeof<CurveLinearColor>()));
        List_sp<RCPtr<CurveLinearColor>> m_colorCurveAssets;

        bool m_isCreatedGpuResource = false;
        array_list<Color4b> m_bitmap;

        gfx::GFXTexture_sp m_gfxTexture;

    };
}