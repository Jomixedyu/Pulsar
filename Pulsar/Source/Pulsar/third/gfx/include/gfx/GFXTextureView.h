#pragma once
#include "GFXInclude.h"
#include "TextureClasses.h"

namespace gfx
{
    class GFXTextureView
    {
    public:
        virtual ~GFXTextureView() = default;
        virtual bool IsWritable() const = 0;
        virtual bool IsReadable() const = 0;
        virtual GFXTextureFormat GetFormat() const = 0;
        virtual GFXTextureTargetType GetTargetType() const = 0;
        virtual int32_t GetWidth() const = 0;
        virtual int32_t GetHeight() const = 0;
        virtual int32_t GetDepth() const { return 1; }
        virtual class GFXTexture* GetTexture() const = 0;
    };
    GFX_DECL_SPTR(GFXTextureView);

    class GFXTexture2DView : public GFXTextureView
    {
    public:
        virtual uint32_t GetBaseArrayIndex() const { return 0; }
    };
    GFX_DECL_SPTR(GFXTexture2DView);

    class GFXTextureCubeView : public GFXTextureView
    {
    public:

        virtual uint32_t GetArrayCount() const { return 1; }
    };
    GFX_DECL_SPTR(GFXTextureCubeView);

}