#pragma once

#include "Texture.h"
#include <Pulsar/IGPUResource.h>
#include <Pulsar/Assets/Texture.h>
#include <gfx/GFXFrameBufferObject.h>
#include <gfx/GFXRenderTarget.h>

namespace pulsar
{
    class Application;

    using RenderTargetType = gfx::GFXRenderTargetType;

    class RenderTexture : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RenderTexture, Texture);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute);
    public:
        RenderTexture();
        ~RenderTexture() override;
        void OnDestroy() override;
    public:
        //Texture
        int32_t GetWidth() const override { return m_width; }
        int32_t GetHeight() const override { return m_height; }
    public:
        
        void PostInitializeData(int32_t width, int32_t height);

        static RCPtr<RenderTexture> StaticCreate(index_string name, int width, int height, int colorRTCount = 1, bool hasDepthStencil = true);
    public:
        //IGPUResource
        bool CreateGPUResource() override;
        void DestroyGPUResource() override;
        bool IsCreatedGPUResource() const override;
        void EnableRenderTarget();
        void DisableRenderTarget();

        const std::shared_ptr<gfx::GFXRenderTarget>& GetGfxRenderTarget0() const { return m_renderTargets[0]; }
        const std::shared_ptr<gfx::GFXFrameBufferObject>& GetGfxFrameBufferObject() const { return m_framebuffer; }
        const array_list<gfx::GFXRenderTarget_sp>& GetRenderTargets() const { return m_renderTargets; }

    protected:

        int32_t m_width;
        int32_t m_height;

        array_list<gfx::GFXRenderTarget_sp> m_renderTargets;
        std::shared_ptr<gfx::GFXFrameBufferObject> m_framebuffer;

        bool m_createdGPUResource = false;
    };
    DECL_PTR(RenderTexture);

}