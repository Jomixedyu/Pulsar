#pragma once
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <gfx/GFXBuffer.h>
#include <memory>

namespace pulsar
{
    class RenderScene;
    class SceneView;
    struct SceneViewData;

    struct SceneCaptureFrameData
    {
        const SceneViewData* view = nullptr;
        SceneView* viewProxy = nullptr;
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
        std::shared_ptr<array_list<rendering::RenderObject_sp>> VisibleRenderers;
    };

    struct SceneRenderTargetFrameData
    {
        RGTextureHandle Target;
    };

    struct SceneOpaqueColorFrameData
    {
        RGTextureHandle Color;
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
