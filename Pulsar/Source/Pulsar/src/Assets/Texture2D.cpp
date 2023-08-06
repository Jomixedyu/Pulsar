#include <Pulsar/Assets/Texture2D.h>
#include <CoreLib/File.h>
#include <Pulsar/Application.h>

namespace pulsar
{
    Texture2D::Texture2D()
    {

    }
    Texture2D::~Texture2D()
    {
        this->UnBindGPU();
    }

    const uint8_t* Texture2D::GetNativeData() const
    {
        return this->m_data;
    }

    void Texture2D::OnInstantiateAsset(AssetObject_ref& obj)
    {
        Texture2D* tex = static_cast<Texture2D*>(obj.GetPtr());

    }

    void Texture2D::InitializeFromPictureMemory(
        const uint8_t* data, int32_t length,
        const SamplerConfig& samplerConfig,
        bool enableReadWrite, TextureFormat format)
    {
        m_tex = Application::GetGfxApp()->CreateTexture2DFromMemory(data, length, samplerConfig, enableReadWrite, format);
    }


    void Texture2D::BindGPU()
    {

        assert(this->GetIsBindGPU());
        //detail::RenderInterface::LoadTexture2D(this->channel_, this->width_, this->height_, this->data_, &this->tex_id_);
    }

    void Texture2D::UnBindGPU()
    {
        if (this->GetIsBindGPU())
        {
            //detail::RenderInterface::UnloadTexture2D(this->tex_id_);
            this->tex_id_ = 0;
        }
    }

    bool Texture2D::GetIsBindGPU()
    {
        return this->tex_id_ != 0;
    }

}
