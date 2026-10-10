#pragma once
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <Pulsar/Rendering/RenderObjectProxy.h>
#include <gfx/GFXBuffer.h>
#include <memory>

namespace gfx { class GFXTexture2DView; }

namespace pulsar
{
    class RenderScene;
    class SceneViewProxy;
    struct SceneViewData;

    struct SceneCaptureFrameData
    {
        const SceneViewData* view = nullptr;
        SceneViewProxy* viewProxy = nullptr;
        RenderScene* scene = nullptr;
        uint64_t frameIndex = 0;
    };

    struct SceneCaptureGpuFrameData
    {
        gfx::GFXBuffer* CameraBuffer = nullptr;
        gfx::GFXBuffer* WorldBuffer = nullptr;
        gfx::GFXBuffer* LightBuffer = nullptr;
        gfx::GFXBuffer* RenderObjectBuffer = nullptr;
    };

    struct SceneViewCullingFrameData
    {
        std::shared_ptr<array_list<rendering::RenderObjectProxy_sp>> VisibleRenderers;
    };

    struct SceneRenderTargetFrameData
    {
        RGTextureHandle Target;
    };

    struct SceneOpaqueColorFrameData
    {
        RGTextureHandle Color;
    };

    struct SceneResolveTargetFrameData
    {
        gfx::GFXTexture2DView* TargetView = nullptr;
    };

    struct ScenePostProcessFrameData
    {
        RGTextureHandle FinalTarget;
        RGTextureHandle ActiveColor;
        RGTextureHandle PingPongA;
        RGTextureHandle PingPongB;

        RGTextureHandle AcquireTarget() const
        {
            return ActiveColor == PingPongA ? PingPongB : PingPongA;
        }

        void PushColor(RGTextureHandle handle)
        {
            ActiveColor = handle;
        }
    };
}
