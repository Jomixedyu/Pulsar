#include "RenderTexture.h"


namespace apatite
{
    RenderTexture::RenderTexture()
    {
    }
    RenderTexture::~RenderTexture()
    {
        this->UnBindGPU();
    }
    void RenderTexture::BindGPU()
    {
        assert(this->GetIsBindGPU());

    }
    void RenderTexture::UnBindGPU()
    {
        if (this->GetIsBindGPU())
        {
            this->tex_id_ = 0;
        }
    }
    bool RenderTexture::GetIsBindGPU()
    {
        return this->tex_id_ != 0;
    }

    int32_t RenderTexture::get_width() const
    {
        return 0;
    }
    int32_t RenderTexture::get_height() const
    {
        return 0;
    }
}