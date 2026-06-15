#pragma once
#include <Pulsar/EngineMath.h>
#include <Pulsar/Assets/RenderTexture.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Subsystems/VolumeStack.h>
#include "RenderProxy.h"
#include <memory>

namespace pulsar
{
    class ScriptableCaptureRenderer;

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
        RCPtr<RenderTexture> RenderTarget;

        // Post-process settings blended for this view's camera position, snapshotted on
        // the game thread (SyncRenderProxy) so the render thread never touches the live
        // PostProcessSubsystem / volume components.
        VolumeStack PostProcessStack;

        // Custom post-process materials collected for this view, snapshotted on the game
        // thread for the same reason.
        array_list<RCPtr<Material>> PostProcessMaterials;
    };

    // Render-thread-owned per-view proxy (capture proxy). Owns the GPU pass pipeline
    // (Renderer) and the current-frame parameter snapshot (Data). One per game-side
    // SceneCaptureComponent.
    class SceneView : public rendering::RenderProxy
    {
    public:
        SceneView() = default;
        ~SceneView() override;

        SceneView(const SceneView&) = delete;
        SceneView& operator=(const SceneView&) = delete;

        SceneViewData                               Data;
        std::unique_ptr<ScriptableCaptureRenderer>  Renderer;
    };
}
