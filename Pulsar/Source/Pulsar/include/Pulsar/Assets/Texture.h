#pragma once
#include <Pulsar/IGPUResource.h>
#include <Pulsar/AssetObject.h>
#include <gfx/GFXTexture.h>

namespace pulsar
{
    using SamplerFilter = gfx::GFXSamplerFilter;
    using TextureFormat = gfx::GFXTextureFormat;
    using SamplerAddressMode = gfx::GFXSamplerAddressMode;
    using SamplerConfig = gfx::GFXSamplerConfig;

    class Texture : public AssetObject, public IGPUResource
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Texture, AssetObject);

    public:
        virtual int32_t GetWidth() const = 0;
        virtual int32_t GetHeight() const = 0;
        virtual Vector2i GetSize2di() const { return { this->GetWidth(), this->GetHeight() }; }
        virtual Vector2f GetSize2df() const { return Vector2f((float)this->GetWidth(), (float)this->GetHeight()); }
    protected:
    };
    DECL_PTR(Texture);
}