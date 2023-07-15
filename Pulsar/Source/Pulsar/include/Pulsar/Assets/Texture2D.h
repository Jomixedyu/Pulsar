#pragma once

#include <Pulsar/AssetObject.h>
#include <Pulsar/IBindGPU.h>
#include "Texture.h"
#include <gfx/GFXTexture2D.h>

namespace pulsar
{


    class Texture2D : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::Texture2D, Texture);
    public:
        Texture2D();
        ~Texture2D() override;
        virtual void SerializeBuildData(ser::Stream& stream, bool is_ser) override;
    public:
        virtual int32_t GetWidth() const override { return m_init ? m_tex->GetWidth() : 0; }
        virtual int32_t GetHeight() const override { return m_init ? m_tex->GetHeight() : 0; }
        int32_t GetChannelCount() const { return m_init ? m_tex->GetChannelCount() : 0; }

    public:
        const uint8_t* GetNativeData() const;
    protected:
        virtual void OnInstantiateAsset(sptr<AssetObject>& obj) override;

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
    public:
        //IBindGPU
        virtual void BindGPU() override;
        virtual void UnBindGPU() override;
        virtual bool GetIsBindGPU() override;
    };
    CORELIB_DECL_SHORTSPTR(Texture2D);
}
