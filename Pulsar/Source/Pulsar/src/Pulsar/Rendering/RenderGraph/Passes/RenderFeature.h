#pragma once
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <Pulsar/Rendering/RenderGraph/ScriptableCaptureRenderer.h>
#include <Pulsar/Subsystems/VolumeStack.h>

namespace pulsar
{
    class CameraComponent;
    class PerPassResources;

    // Generic render feature interface.
    // Any render logic that can be injected into the pipeline (post-processing, SSAO, SSR, etc.)
    // implements this interface. The DefaultSceneCaptureRenderer iterates over registered features
    // without knowing their concrete types.
    class RenderFeature
    {
    public:
        virtual ~RenderFeature() = default;

        // Called before AddToGraph so the feature can query world/camera data.
        virtual void OnSetup(const RenderCaptureContext& ctx) {}

        // Called before AddToGraph so the feature can pull its own settings from the stack.
        virtual void ReadSettings(const VolumeStack& stack) {}

        // Whether this feature should be injected into the render graph this frame.
        virtual bool IsEnabled() const { return true; }

        // Inject this feature into the render graph.
        // 'input' is the current scene color handle.
        // 'output' is a writable target (usually the ping-pong buffer).
        // Returns the handle that holds the result after this feature.
        virtual RGTextureHandle AddToGraph(RenderGraph& graph,
                                           RGTextureHandle input,
                                           RGTextureHandle output,
                                           CameraComponent* cam,
                                           PerPassResources* perPass) = 0;
    };

} // namespace pulsar
