#pragma once

#include <Apatite/AssetObject.h>
#include "Texture.h"

namespace apatite
{
    class Texture2D : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::Texture2D, Texture);
    public:
        Texture2D();
        ~Texture2D() override;
        virtual void Serialize(ser::Stream& stream, bool is_ser) override;
    public:
        int32_t get_width() const { return width_; }
        int32_t get_height() const { return height_; }
        int32_t get_channel_count() const { return channel_; }

    public:
        const uint8_t* GetNativeData() const;
    protected:
        virtual void OnInstantiateAsset(sptr<AssetObject>& obj) override;
    protected:
        uint8_t* data_;
        int32_t width_;
        int32_t height_;
        int32_t channel_;
    };
    CORELIB_DECL_SHORTSPTR(Texture2D);
}
