#pragma once

#include "Texture.h"
#include <Pulsar/IGPUResource.h>
#include <Pulsar/AssetObject.h>
#include <gfx/GFXTexture2D.h>

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
        virtual int32_t GetWidth() const override { return m_init ? m_tex->GetWidth() : 0; }
        virtual int32_t GetHeight() const override { return m_init ? m_tex->GetHeight() : 0; }
        int32_t GetChannelCount() const { return m_init ? m_tex->GetChannelCount() : 0; }

    public:
        const uint8_t* GetNativeData() const;
    protected:
        virtual void OnInstantiateAsset(AssetObject* obj) override;

    public:
        void InitializeFromPictureMemory(const uint8_t* data, int32_t length, const SamplerConfig& samplerConfig, bool enableReadWrite, TextureFormat format);
    protected:

        uint8_t* m_data = nullptr;
        size_t n_dataLength;
        gfx::GFXSamplerConfig m_samplerConfig;
        bool m_enableReadWrite;
        gfx::GFXTextureFormat m_format;

        std::shared_ptr<gfx::GFXTexture2D> m_tex;
        bool m_init = false;

#ifdef WITH_EDITOR
        BinaryData m_originalImageData;
#endif // WITH_EDITOR

    public:
        //IGPUResource
        virtual void CreateGPUResource() override;
        virtual void DestroyGPUResource() override;
        virtual bool IsCreatedGPUResource() const override;
    };
    DECL_PTR(Texture2D);
}
