#pragma once
#include <Pulsar/AssetObject.h>
#include <Pulsar/IBindGPU.h>
#include <gfx/GFXTexture.h>

namespace pulsar
{
    using SamplerFilter = gfx::GFXSamplerFilter;
    using TextureFormat = gfx::GFXTextureFormat;
    using SamplerAddressMode = gfx::GFXSamplerAddressMode;
    using SamplerConfig = gfx::GFXSamplerConfig;

    class Texture : public AssetObject, public IBindGPU
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::Texture, AssetObject);

    public:
        virtual int32_t GetWidth() const = 0;
        virtual int32_t GetHeight() const = 0;
        virtual Vector2i GetSize2di() const { return { this->GetWidth(), this->GetHeight() }; }
        virtual Vector2f GetSize2df() const { return Vector2f(this->GetWidth(), this->GetHeight()); }
    protected:
        uint32_t tex_id_ = 0;
    };
    CORELIB_DECL_SHORTSPTR(Texture);
}