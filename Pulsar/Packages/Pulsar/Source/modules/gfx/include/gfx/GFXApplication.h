#pragma once
#include "GFXApi.h"
#include "GFXExtensions.h"
#include "GFXGlobalConfig.h"
#include "GFXGraphicsPipelineManager.h"
#include "GFXRenderPipeline.h"
#include "GFXSwapchain.h"
#include "GFXTextureView.h"
#include "GFXVertexLayoutDescription.h"
#include "GFXSurface.h"
#include "GFXSampler.h"
#include <functional>

namespace gfx
{
    class GFXResourceRegistry;

    class GFXApplication
    {
    public:
        GFXApplication(const GFXApplication&) = delete;
        GFXApplication(GFXApplication&&) = delete;
        virtual ~GFXApplication();

        virtual void Initialize()
        {
        }
        virtual void Terminate()
        {
        }

        // Block until the GPU has finished all submitted work.
        virtual void WaitDeviceIdle() = 0;

        const GFXGlobalConfig& GetConfig() const
        {
            return m_config;
        }
        virtual GFXExtensions GetExtensionNames() = 0;
        virtual GFXApi GetApiType() const = 0;
        virtual const char* GetApiLevelName() const = 0;

        virtual void SetRenderPipeline(GFXRenderPipeline* pipeline) = 0;
        virtual GFXRenderPipeline* GetRenderPipeline() const = 0;

    public:

        virtual GFXGraphicsPipelineManager* GetGraphicsPipelineManager() const = 0;

        virtual array_list<GFXTextureFormat> GetSupportedDepthFormats() = 0;

        virtual std::vector<uint8_t> ReadbackTexture(GFXTexture* texture, int32_t width, int32_t height) = 0;

        virtual intptr_t GetWindowHandle() = 0;
        virtual GFXSurface* GetWindow() = 0;

        virtual GFXSwapchain* GetViewport() = 0;

        GFXResourceRegistry* GetResourceRegistry() const { return m_resourceRegistry; }

        // Frame-phase hook for profiling: invoked as (name, begin/end) around each
        // phase of the backend frame (acquire, record, wait, submit, present). The
        // backend knows nothing about the profiler; the engine bridges it. Set once at
        // startup; invoked on the render thread.
        using GFXFramePhaseFn = std::function<void(std::string_view name, bool begin)>;
        void SetFramePhaseCallback(GFXFramePhaseFn fn) { m_framePhaseFn = std::move(fn); }

        // Number of per-frame resource slots (one frame recording + one executing on
        // the GPU). Not the GPU in-flight depth: that is 1 (a frame is submitted only
        // after the previous one completed).
        virtual uint32_t GetFrameSlotCount() const { return 2; }

        // The unified GPU timeline: every frame's submission is one node, identified by
        // its 1-based index. GetFrameCount() = nodes submitted so far;
        // GetCompletedTimeline() = nodes provably finished on the GPU. A resource
        // referenced by node N may be destroyed/reused once GetCompletedTimeline() >= N.
        virtual uint64_t GetFrameCount() const { return 0; }
        virtual uint64_t GetCompletedTimeline() const { return GetFrameCount(); }

        // Current frame slot (frame count % GetFrameSlotCount()). Use it to
        // index per-slot copies of anything rewritten during recording.
        virtual uint32_t GetFrameInFlightIndex() const { return 0; }

    protected:
        GFXApplication() = default;

    protected:
        GFXGlobalConfig m_config{};
        GFXResourceRegistry* m_resourceRegistry = nullptr;
        GFXFramePhaseFn m_framePhaseFn;
    };

} // namespace gfx
