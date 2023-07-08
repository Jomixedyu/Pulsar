#pragma once

#include <Pulsar/AssetObject.h>
#include <Pulsar/IBindGPU.h>
#include "Texture.h"

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
        virtual int32_t get_width() const override { return width_; }
        virtual int32_t get_height() const override { return height_; }
        int32_t get_channel_count() const { return channel_; }

    public:
        const uint8_t* GetNativeData() const;
    protected:
        virtual void OnInstantiateAsset(sptr<AssetObject>& obj) override;
    protected:
        uint8_t* data_ = nullptr;
        int32_t width_ = 0;
        int32_t height_ = 0;
        int32_t channel_ = 0;
    public:
        //IBindGPU
        virtual void BindGPU() override;
        virtual void UnBindGPU() override;
        virtual bool GetIsBindGPU() override;
    };
    CORELIB_DECL_SHORTSPTR(Texture2D);
}
