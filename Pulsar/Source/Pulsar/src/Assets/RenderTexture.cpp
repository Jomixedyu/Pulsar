#include "Assets/RenderTexture.h"
#include <Pulsar/Application.h>

namespace pulsar
{
    RenderTexture::RenderTexture()
    {
    }


    RenderTexture::~RenderTexture()
    {
    }
    void RenderTexture::OnDestroy()
    {
        base::OnDestroy();
        DestroyGPUResource();
    }
    bool RenderTexture::CreateGPUResource()
    {
        if (IsCreatedGPUResource())
        {
            return true;
        }
        m_createdGPUResource = true;
        return true;
    }
    void RenderTexture::DestroyGPUResource()
    {
        if (!IsCreatedGPUResource())
        {
            return;
        }
        m_createdGPUResource = false;
    }
    bool RenderTexture::IsCreatedGPUResource() const
    {
        return m_createdGPUResource;
    }

    void RenderTexture::EnableRenderTarget()
    {
    }

    void RenderTexture::DisableRenderTarget()
    {
    }




    RCPtr<RenderTexture> RenderTexture::StaticCreate(index_string name, int width, int height,
        const array_list<RenderTargetInfo>& targetInfo)
    {
        auto self = mksptr(new RenderTexture);
        self->Construct();
        self->SetIndexName(name);

        auto gfx = Application::GetGfxApp();

        auto depthFormats = gfx->GetSupportedDepthFormats();
        assert(!depthFormats.empty());

        for (auto& info : targetInfo)
        {
            auto rt = gfx->CreateRenderTarget(width, height, info.TargetType, info.Format, {});
            self->m_renderTargets.push_back(rt);
        }

        std::vector<gfx::GFXTexture2DView_sp> rts;
        std::vector<gfx::GFXTexture2DView*> rtsPtr;
        for (auto& rt : self->m_renderTargets)
        {
            auto view = rt->Get2DView(0);
            rts.push_back(view);
            rtsPtr.push_back(view.get());
        }

        auto renderPass = gfx->CreateRenderPassLayout(rtsPtr);
        self->m_framebuffer = gfx->CreateFrameBufferObject(rts, renderPass);

        self->m_width = width;
        self->m_height = height;

        return self;
    }

    void RenderTexture::PostInitializeData(int32_t width, int32_t height)
    {
    }
    array_list<gfx::GFXTextureFormat> RenderTexture::GetSupportedDepthFormats()
    {
        auto gfx = Application::GetGfxApp();
        return gfx->GetSupportedDepthFormats();
    }
    bool RenderTexture::IsSupportedDepthFormat(gfx::GFXTextureFormat format)
    {
        for (auto element : GetSupportedDepthFormats())
        {
            if (element == format)
                return true;
        }
        return false;
    }

} // namespace pulsar