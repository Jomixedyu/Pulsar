#pragma once

#include "Texture.h"
#include <Pulsar/IGPUResource.h>
#include <Pulsar/AssetObject.h>
#include <gfx/GFXTexture.h>




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
        CORELIB_CLASS_ATTR(new AssetIconAttribute("Editor/Icons/texture.png"))
    public:
        Texture2D();
        ~Texture2D() override;
    public:
        void Serialize(AssetSerializer* s) override;
        virtual int32_t GetWidth() const override { return m_textureSize.x; }
        virtual int32_t GetHeight() const override { return m_textureSize.y; }

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

        std::shared_ptr<gfx::GFXTexture> GetGFXTexture() const override { return m_tex; }

    public:
        bool IsSRGB() const
        {
            auto fmt = StaticGetFormatMapping(OSPlatform::Windows64)->at(m_compressionFormat);
            return fmt == gfx::GFXTextureFormat::BC3_SRGB || fmt == gfx::GFXTextureFormat::R8G8B8A8_SRGB;
        }
        void SetIsSRGB(bool) {} // 已废弃，CompressionFormat 决定 sRGB/Linear
        TextureCompressionFormat GetCompressedFormat() const override { return m_compressionFormat; }
        void SetCompressedFormat(TextureCompressionFormat value) { m_compressionFormat = value; }
        size_t GetOriginCompressedBinarySize() const override { return m_originMemory.size(); }
        size_t GetRawBinarySize() const override { return m_cachedUncompressedRawSize; }
        size_t GetNativeBinarySize() const override { return m_cachedNativeSize; }

        const uint8_t* GetOriginMemoryData() const { return m_originMemory.data(); }
        bool IsOriginMemoryCompressed() const { return m_compressedOriginImage; }
        bool IsOriginMemoryLoaded() const { return m_loadedOriginMemory; }
    protected:

        bool m_isSRGB{}; // 已废弃，保留字段以兼容旧资产序列化

        array_list<uint8_t> m_originMemory;
        bool m_compressedOriginImage = false;
        bool m_loadedOriginMemory = false;
        size_t m_cachedUncompressedRawSize{};
        size_t m_cachedNativeSize{};

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
