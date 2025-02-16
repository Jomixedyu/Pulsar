#pragma once
#include <Pulsar/IGPUResource.h>
#include <Pulsar/AssetObject.h>
#include <gfx/GFXTexture.h>

namespace pulsar
{
    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar,
        TextureCompressionFormat,
        ColorSRGB_Compressed,
        BitmapRGBA,
        Gray,
        NormalMap_Compressed,
        HDR_Compressed
        );

    using SamplerFilter = gfx::GFXSamplerFilter;
    CORELIB_DEF_BOXING_ENUM_EXTERN(AssemblyObject_pulsar, pulsar, SamplerFilter, gfx::BuildEnumMap_SamplerFilter);
    using TextureFormat = gfx::GFXTextureFormat;
    using SamplerAddressMode = gfx::GFXSamplerAddressMode;
    CORELIB_DEF_BOXING_ENUM_EXTERN(AssemblyObject_pulsar, pulsar, SamplerAddressMode, gfx::BuildEnumMap_SamplerAddressMode);
    using SamplerConfig = gfx::GFXSamplerConfig;

}
CORELIB_DECL_BOXING(pulsar::TextureCompressionFormat, pulsar::BoxingTextureCompressionFormat);
CORELIB_DECL_BOXING(pulsar::SamplerFilter, pulsar::BoxingSamplerFilter);
CORELIB_DECL_BOXING(pulsar::SamplerAddressMode, pulsar::BoxingSamplerAddressMode);

namespace pulsar
{

    class Texture : public AssetObject, public IGPUResource
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Texture, AssetObject);

    public:
        static hash_map<TextureCompressionFormat, gfx::GFXTextureFormat>* StaticGetFormatMapping(OSPlatform platform);

        virtual int32_t GetWidth() const = 0;
        virtual int32_t GetHeight() const = 0;
        virtual Vector2i GetSize2di() const { return { this->GetWidth(), this->GetHeight() }; }
        virtual Vector2f GetSize2df() const { return Vector2f((float)this->GetWidth(), (float)this->GetHeight()); }
        virtual std::shared_ptr<gfx::GFXTexture> GetGFXTexture() const { return nullptr; }
        virtual TextureCompressionFormat GetCompressedFormat() const { return TextureCompressionFormat::BitmapRGBA; }
        virtual size_t GetOriginCompressedBinarySize() const { return 0; }
        virtual size_t GetRawBinarySize() const { return 0; }
        virtual size_t GetNativeBinarySize() const { return 0; }

        SamplerFilter GetSamplerFilter() const { return m_samplerFilter; }
        SamplerAddressMode GetSamplerAddressMode() const { return m_samplerAddressMode; }

    protected:
        CORELIB_REFL_DECL_FIELD(m_samplerAddressMode);
        SamplerAddressMode m_samplerAddressMode{};

        CORELIB_REFL_DECL_FIELD(m_samplerFilter);
        SamplerFilter m_samplerFilter{};
    };
    DECL_PTR(Texture);
}