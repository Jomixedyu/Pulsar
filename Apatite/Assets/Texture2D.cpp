#include <Apatite/Assets/Texture2D.h>
#include <Apatite/Private/RenderInterface.h>
#include <Apatite/Private/ResourceInterface.h>
#include <CoreLib/File.h>

namespace apatite
{
    Texture2D::Texture2D()
    {

    }
    Texture2D::~Texture2D()
    {
        this->UnBindGPU();
    }

    void Texture2D::Serialize(ser::Stream& stream, bool is_ser)
    {
        ser::ReadWriteStream(stream, is_ser, this->width_);
        ser::ReadWriteStream(stream, is_ser, this->height_);
        ser::ReadWriteStream(stream, is_ser, this->channel_);
        int32_t len = this->width_ * this->height_;
        ser::ReadWriteStream(stream, is_ser, len);
        if (is_ser)
        {
            stream.WriteBytes(this->data_, 0, len);
        }
        else
        {
            this->data_ = new uint8_t[len];
            stream.ReadBytes(this->data_, 0, len);
            
        }
    }



    const uint8_t* Texture2D::GetNativeData() const
    {
        return this->data_;
    }

    void Texture2D::OnInstantiateAsset(sptr<AssetObject>& obj)
    {
        Texture2D* tex = static_cast<Texture2D*>(obj.get());
        tex->data_ = this->data_;
        tex->channel_ = this->channel_;
        tex->height_ = this->height_;
        tex->width_ = this->width_;
    }

    void Texture2D::BindGPU()
    {
        assert(this->GetIsBindGPU());
        detail::RenderInterface::LoadTexture2D(this->channel_, this->width_, this->height_, this->data_, &this->tex_id_);
    }

    void Texture2D::UnBindGPU()
    {
        if (this->GetIsBindGPU())
        {
            detail::RenderInterface::UnloadTexture2D(this->tex_id_);
            this->tex_id_ = 0;
        }
    }

    bool Texture2D::GetIsBindGPU()
    {
        return this->tex_id_ != 0;
    }

}
