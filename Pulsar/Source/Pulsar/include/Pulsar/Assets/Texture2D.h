#pragma once

#include "Texture.h"
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

}
CORELIB_DECL_BOXING(pulsar::TextureCompressionFormat, pulsar::BoxingTextureCompressionFormat);

namespace pulsar
{
    class BinaryData
    {
        array_list<uint8_t> m_buffer;
    public:
        void Fill(uint8_t* data, size_t size)
        {
            m_buffer.resize(size);
            memcpy(m_buffer.data(), data, size);
        }
        uint8_t* GetData() { return m_buffer.data(); }
        const uint8_t* GetData() const { return m_buffer.data(); }
        size_t Size() const { return m_buffer.size(); }
    };



    class Texture2D : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Texture2D, Texture);
    public:
        Texture2D();
        ~Texture2D() override;
    public:
        void Serialize(AssetSerializer* s) override;
        virtual int32_t GetWidth() const override { return m_textureSize.x; }
        virtual int32_t GetHeight() const override { return m_textureSize.y; }
        static hash_map<TextureCompressionFormat, gfx::GFXTextureFormat>* StaticGetFormatMapping(OSPlatform platform);
    public:
        void OnDestroy() override;
    protected:
        virtual void OnInstantiateAsset(AssetObject* obj) override;

    public:
        void FromNativeData(const uint8_t* data, size_t length, bool compressed, int width, int height, int channel);
        void PostEditChange(FieldInfo* info) override;
    public:
        //IGPUResource
        bool CreateGPUResource() override;
        void DestroyGPUResource() override;
        bool IsCreatedGPUResource() const override;

        std::shared_ptr<gfx::GFXTexture> GetGFXTexture() const { return m_tex; }

    public:
        bool IsSRGB() const { return m_isSRGB; }
        void SetIsSRGB(bool value) { m_isSRGB = value; }
        auto GetCompressedFormat() const { return m_compressionFormat; }
        size_t GetOriginCompressedBinarySize() const { return m_originMemory.size(); }
        size_t GetRawBinarySize() const { return m_cachedUncompressedRawSize; }
        size_t GetNativeBinarySize() const { return m_cachedNativeSize; }
    protected:

        CORELIB_REFL_DECL_FIELD(m_isSRGB);
        bool m_isSRGB;

        array_list<uint8_t> m_originMemory;
        bool m_compressedOriginImage = false;
        bool m_loadedOriginMemory = false;
        size_t m_cachedUncompressedRawSize{};
        size_t m_cachedNativeSize{};

        gfx::GFXSamplerConfig m_samplerConfig{};
        bool m_enableReadWrite{};
        gfx::GFXTextureFormat m_format{};

        std::shared_ptr<gfx::GFXTexture> m_tex;
        bool m_init = false;

        bool m_isCreatedGPUResource = false;

        CORELIB_REFL_DECL_FIELD(m_compressionFormat);
        TextureCompressionFormat m_compressionFormat{};

        Vector2i m_textureSize{};
        int m_channelCount{};

    };
    DECL_PTR(Texture2D);
}
