#pragma once
#include "GFXApi.h"
#include "GFXBuffer.h"
#include "GFXCommandBuffer.h"
#include "GFXDescriptorSet.h"
#include "GFXExtensions.h"
#include "GFXGlobalConfig.h"
#include "GFXGpuProgram.h"
#include "GFXGraphicsPipelineManager.h"
#include "GFXInclude.h"
#include "GFXRenderPass.h"
#include "GFXRenderPipeline.h"
#include "GFXRenderer.h"
#include "GFXSwapchain.h"
#include "GFXTextureView.h"
#include "GFXVertexLayoutDescription.h"
#include "GFXSurface.h"
#include "GFXSampler.h"
#include <functional>
#include <memory>

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

        // Queue a staging->device copy of `size` bytes from `data` into `dst` at `dstOffset`, batched
        // and flushed at the next frame head (no per-call submit/wait). Must be called on the render
        // thread between the frame's resource-update drain and that frame's render submit.
        virtual void RequestBufferUpload(GFXBuffer* dst, const void* data, size_t size, size_t dstOffset = 0) = 0;

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

        virtual GFXRenderer* GetRenderer() = 0;

        virtual GFXCommandBufferPtr CreateCommandBuffer() = 0;

        virtual GFXGraphicsPipelineManager* GetGraphicsPipelineManager() const = 0;

        virtual array_list<GFXTextureFormat> GetSupportedDepthFormats() = 0;

        virtual std::vector<uint8_t> ReadbackTexture(GFXTexture* texture, int32_t width, int32_t height) = 0;

        virtual intptr_t GetWindowHandle() = 0;
        virtual GFXSurface* GetWindow() = 0;

        virtual GFXSwapchain* GetViewport() = 0;

        GFXResourceRegistry* GetResourceRegistry() const { return m_resourceRegistry; }

    protected:
        GFXApplication() = default;

    protected:
        GFXGlobalConfig m_config{};
        GFXResourceRegistry* m_resourceRegistry = nullptr;
    };

} // namespace gfx
