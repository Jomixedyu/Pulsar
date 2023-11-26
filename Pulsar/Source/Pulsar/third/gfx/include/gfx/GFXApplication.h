#pragma once
#include "GFXGlobalConfig.h"
#include "GFXExtensions.h"
#include "GFXBuffer.h"
#include "GFXCommandBuffer.h"
#include "GFXTexture2D.h"
#include "GFXVertexLayoutDescription.h"
#include "GFXDescriptorManager.h"
#include "GFXGpuProgram.h"
#include "GFXShaderPass.h"
#include "GFXRenderTarget.h"
#include "GFXViewport.h"
#include "GFXRenderPipeline.h"
#include "GFXRenderPass.h"
#include "GFXGraphicsPipelineManager.h"
#include "GFXInclude.h"
#include "GFXApi.h"
#include <functional>

namespace gfx
{

    class GFXApplication
    {
    public:
        using LoopEvent = std::function<void(float)>;

        //using LoopEvent = void(*)(GFXApplication*, float);
        using ExitWindowEvent = bool(*)();
    public:
        GFXApplication(const GFXApplication&) = delete;
        GFXApplication(GFXApplication&&) = delete;


        virtual void Initialize() {}
        virtual void ExecLoop() {}
        virtual void RequestStop() {}
        virtual void Terminate() {}

        const GFXGlobalConfig& GetConfig() const { return m_config; }
        virtual GFXExtensions GetExtensionNames() = 0;
        virtual GFXApi GetApiType() const = 0;
        virtual const char* GetApiLevelName() const = 0;

        virtual void SetRenderPipeline(GFXRenderPipeline* pipeline) = 0;
        virtual GFXRenderPipeline* GetRenderPipeline() const = 0;

        LoopEvent OnPreRender = nullptr;
        LoopEvent OnPostRender = nullptr;
        ExitWindowEvent OnExitWindow = nullptr;
    public:
        virtual GFXBuffer_sp CreateBuffer(GFXBufferUsage usage, size_t bufferSize) = 0;
        virtual GFXCommandBuffer_sp CreateCommandBuffer() = 0;
        virtual GFXVertexLayoutDescription_sp CreateVertexLayoutDescription() = 0;
        virtual GFXGpuProgram_sp CreateGpuProgram(const std::unordered_map<gfx::GFXShaderStageFlags, array_list<char>>& codes) = 0;
        virtual GFXShaderPass_sp CreateShaderPass(
            const GFXShaderPassConfig& config,
            const GFXGpuProgram_sp& gpuProgram,
            const GFXDescriptorSetLayout_sp& descSetLayout,
            const array_list<GFXVertexLayoutDescription_sp>& vertexLayout) = 0;

        virtual GFXDescriptorManager* GetDescriptorManager() = 0;
        
        virtual GFXDescriptorSetLayout_sp CreateDescriptorSetLayout(
            const array_list<GFXDescriptorSetLayoutInfo>& layoutInfos) = 0;

        virtual GFXGraphicsPipelineManager* GetGraphicsPipelineManager() const = 0;

        virtual GFXTexture2D_sp CreateFromImageData(
            const uint8_t* imageData, int32_t width, int32_t height, int32_t channel,
            bool enableReadWrite, GFXTextureFormat format, const GFXSamplerConfig& samplerCfg) = 0;

        virtual GFXTexture2D_sp CreateTexture2DFromMemory(
            const uint8_t* data, int32_t length,
            const GFXSamplerConfig& samplerConfig,
            bool enableReadWrite = false, 
            GFXTextureFormat format = GFXTextureFormat::R8G8B8A8_SRGB) = 0;

        virtual GFXRenderTarget_sp CreateRenderTarget(
            int32_t width, int32_t height, GFXRenderTargetType type,
            GFXTextureFormat format, const GFXSamplerConfig& samplerCfg) = 0;

        virtual GFXFrameBufferObject_sp CreateFrameBufferObject(
            const array_list<GFXRenderTarget*>& renderTargets,
            const GFXRenderPassLayout_sp& renderPassLayout) = 0;

        virtual GFXRenderPassLayout_sp CreateRenderPassLayout(const array_list<GFXRenderTarget*>& renderTargets) = 0;

        virtual array_list<GFXTextureFormat> GetSupportedDepthFormats() = 0;

        virtual intptr_t GetWindowHandle() = 0;

        virtual GFXViewport* GetViewport() = 0;
    protected:
        GFXApplication() {}
    protected:
        GFXGlobalConfig m_config{};
    };

}