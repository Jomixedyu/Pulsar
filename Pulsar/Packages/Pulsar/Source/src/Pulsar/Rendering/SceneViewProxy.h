#pragma once
#include <Pulsar/EngineMath.h>
#include <Pulsar/Subsystems/VolumeStack.h>
#include "SceneProxy.h"
#include "PerPassData.h"
#include "RenderTextureProxy.h"
#include "RenderGraph/Pipelines/ViewPipelineRenderData.h"
#include <gfx/GFXTexture.h>
#include <gfx/GFXFrameBufferObject.h>
#include <gfx/GFXBuffer.h>
#include <memory>

namespace pulsar
{
    class ViewPipeline;

    // A view's render target, referenced by proxy. GPU objects (textures/FBO) are
    // resolved on the render thread per frame: the proxy initializes and rebuilds
    // asynchronously, so caching its gfx objects here would go stale.
    struct RenderTargetSnapshot
    {
        std::shared_ptr<rendering::RenderTextureProxy> Proxy;

        bool IsValid() const { return Proxy && Proxy->IsInitialized(); }
        int32_t GetWidth() const { return Proxy ? Proxy->GetWidth() : 0; }
        int32_t GetHeight() const { return Proxy ? Proxy->GetHeight() : 0; }
        gfx::GFXTexture2DViewPtr GetColorTextureView() const
        {
            return Proxy ? Proxy->GetColorTextureView() : nullptr;
        }
    };

    // Per-view parameter snapshot, extracted from the game-side capture component
    // at the end of World::Tick and written on the render thread. The render path
    // reads these instead of touching the component.
    struct SceneViewData
    {
        Matrix4f ViewMatrix{};
        Matrix4f ProjectionMatrix{};
        Vector3f CameraPosition{};
        Vector3f CameraForward{};
        float    Near = 0.f;
        float    Far  = 0.f;
        Vector2f Resolution{};
        Color4f  BackgroundColor{};
        uint32_t MSAASamples = 1;
        bool     GizmoPassEnabled = false;
        ViewPipelineRenderData ViewPipeline;

        // Render target resolved to gfx resources on the game thread (no RCPtr).
        RenderTargetSnapshot RenderTarget;

        // Post-process settings blended for this view's camera position, snapshotted on
        // the game thread (ResolveRenderStateDirty) so the render thread never touches the live
        // PostProcessSubsystem / volume components.
        VolumeStack PostProcessStack;

    };

    // Render-thread-owned per-view proxy (capture proxy). Owns the view pipeline
    // (Pipeline) and the current-frame parameter snapshot (Data). One per game-side
    // SceneCaptureComponent.
    class SceneViewProxy : public rendering::SceneProxy
    {
    public:
        SceneViewProxy();
        ~SceneViewProxy() override;

        SceneViewProxy(const SceneViewProxy&) = delete;
        SceneViewProxy& operator=(const SceneViewProxy&) = delete;

        void OnEnterScene() override;

        void OnLeaveScene() override;

        // Per-view camera cbuffer (owned here). Lazily created on first upload
        // (render thread, GFX alive).
        void UploadCamera(const PerPassCameraData& data);
        gfx::GFXBuffer* GetCameraBuffer() const;
        void SetData(SceneViewData data);


        SceneViewData                               Data;
        std::unique_ptr<ViewPipeline>                Pipeline;

    private:
        gfx::GFXBufferPtr m_cameraBuffer;
    };
}
