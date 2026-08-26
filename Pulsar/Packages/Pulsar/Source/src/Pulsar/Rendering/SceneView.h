#pragma once
#include <Pulsar/EngineMath.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Subsystems/VolumeStack.h>
#include "RenderProxy.h"
#include "PerPassData.h"
#include "RenderGraph/Pipelines/ViewPipelineRenderData.h"
#include <gfx/GFXTexture.h>
#include <gfx/GFXFrameBufferObject.h>
#include <gfx/GFXBuffer.h>
#include <memory>

namespace pulsar
{
    class ViewPipeline;

    // GFX resource bundle for a view's render target, resolved on the game thread from
    // the RenderTexture asset. Holds only gfx shared resources (no RCPtr / AssetObject),
    // so it is safe for the render thread to read and to destruct across threads.
    struct RenderTargetSnapshot
    {
        int32_t Width = 0;
        int32_t Height = 0;
        array_list<gfx::GFXTexture_sp>          Attachments;
        gfx::GFXFrameBufferObject_sp            Framebuffer;

        bool IsValid() const { return Framebuffer != nullptr; }
        gfx::GFXTexture2DView_sp GetRenderTarget0() const
        {
            if (Attachments.empty()) return nullptr;
            return Attachments[0]->Get2DView(0);
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

        // Custom post-process materials collected for this view, snapshotted on the game
        // thread for the same reason.
        array_list<RCPtr<Material>> PostProcessMaterials;
    };

    // Render-thread-owned per-view proxy (capture proxy). Owns the view pipeline
    // (Pipeline) and the current-frame parameter snapshot (Data). One per game-side
    // SceneCaptureComponent.
    class SceneView : public rendering::RenderProxy
    {
    public:
        SceneView();
        ~SceneView() override;

        SceneView(const SceneView&) = delete;
        SceneView& operator=(const SceneView&) = delete;

        void OnCreateResource() override;

        void OnDestroyResource() override;

        // Per-view camera cbuffer (owned here). Lazily created on first upload
        // (render thread, GFX alive).
        void UploadCamera(const PerPassCameraData& data);
        gfx::GFXBuffer* GetCameraBuffer() const;
        void SetData(SceneViewData data);


        SceneViewData                               Data;
        std::unique_ptr<ViewPipeline>                Pipeline;

    private:
        gfx::GFXBuffer_sp m_cameraBuffer;
    };
}
