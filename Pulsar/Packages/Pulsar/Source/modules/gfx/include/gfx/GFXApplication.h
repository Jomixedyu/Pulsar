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
#include "GFXGlobalShaderManager.h"
#include "GFXSurface.h"
#include "GFXResourceManager.h"
#include "GFXSampler.h"
#include "GFXBuiltinResources.h"
#include <functional>
#include <memory>

namespace gfx
{

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
            m_shaderManager.Clear();
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

        virtual GFXBuffer_sp CreateBuffer(const GFXBufferDesc& desc) = 0;
        virtual GFXCommandBuffer_sp CreateCommandBuffer() = 0;
        virtual GFXGpuProgram_sp CreateGpuProgram(GFXGpuProgramStageFlags stage, const uint8_t* code, size_t length) = 0;

        virtual GFXDescriptorSetLayout_sp GetOrCreateDescriptorSetLayout(
            const GFXDescriptorLayoutDesc* layouts,
            size_t layoutCount) = 0;
        virtual GFXDescriptorSetLayout_sp GetOrCreateDescriptorSetLayout(
            std::initializer_list<GFXDescriptorLayoutDesc> layouts);

        virtual GFXGraphicsPipelineManager* GetGraphicsPipelineManager() const = 0;


        virtual GFXTexture_sp CreateTexture2DFromMemory(
            const uint8_t* imageData, size_t length,
            int width, int height,
            GFXTextureFormat format,
            const GFXSamplerConfig& samplerConfig
            ) = 0;

        virtual GFXTexture_sp CreateTextureCube(int32_t size) = 0;

        virtual GFXSampler_sp CreateSampler(const GFXSamplerConfig& config) = 0;

        virtual GFXTexture_sp CreateRenderTarget(
            int32_t width, int32_t height, GFXTextureTargetType type,
            GFXTextureFormat format, const GFXSamplerConfig& samplerCfg,
            uint32_t sampleCount = 1, bool isTransientAttachment = false) = 0;

        virtual GFXFrameBufferObject_sp CreateFrameBufferObject(
            const array_list<GFXTexture2DView_sp>& renderTargets) = 0;

        virtual array_list<GFXTextureFormat> GetSupportedDepthFormats() = 0;

        virtual std::vector<uint8_t> ReadbackTexture(GFXTexture* texture, int32_t width, int32_t height) = 0;

        virtual intptr_t GetWindowHandle() = 0;
        virtual GFXSurface* GetWindow() = 0;

        virtual GFXSwapchain* GetViewport() = 0;

        GFXResourceManager* GetResourceManager() const { return m_resourceManager.get(); }

        GFXBuiltinResources& GetBuiltinResources() { return m_builtinResources; }

        GFXGlobalShaderManager& GetGlobalShaderManager() { return m_shaderManager; }

    protected:
        GFXApplication() = default;

    protected:
        GFXGlobalConfig m_config{};
        GFXGlobalShaderManager m_shaderManager;
        std::unique_ptr<GFXResourceManager> m_resourceManager;
        GFXBuiltinResources m_builtinResources;
    };

} // namespace gfx