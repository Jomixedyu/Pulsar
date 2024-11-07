#pragma once
#include "GFXApi.h"
#include "GFXBuffer.h"
#include "GFXCommandBuffer.h"
#include "GFXDescriptorManager.h"
#include "GFXExtensions.h"
#include "GFXGlobalConfig.h"
#include "GFXGpuProgram.h"
#include "GFXGraphicsPipelineManager.h"
#include "GFXInclude.h"
#include "GFXRenderPass.h"
#include "GFXRenderPipeline.h"
#include "GFXRenderer.h"
#include "GFXShaderPass.h"
#include "GFXTextureView.h"
#include "GFXVertexLayoutDescription.h"
#include "GFXViewport.h"
#include <functional>

namespace gfx
{

    class GFXApplication
    {
    public:
        using LoopEvent = std::function<void(float)>;

        // using LoopEvent = void(*)(GFXApplication*, float);
        using ExitWindowEvent = bool (*)();

    public:
        GFXApplication(const GFXApplication&) = delete;
        GFXApplication(GFXApplication&&) = delete;
        virtual ~GFXApplication()  = default;

        virtual void Initialize()
        {
        }
        virtual void ExecLoop()
        {
        }
        virtual void RequestStop()
        {
        }
        virtual void Terminate()
        {
        }

        const GFXGlobalConfig& GetConfig() const
        {
            return m_config;
        }
        virtual GFXExtensions GetExtensionNames() = 0;
        virtual GFXApi GetApiType() const = 0;
        virtual const char* GetApiLevelName() const = 0;

        virtual void SetRenderPipeline(GFXRenderPipeline* pipeline) = 0;
        virtual GFXRenderPipeline* GetRenderPipeline() const = 0;

        LoopEvent OnPreRender = nullptr;
        LoopEvent OnPostRender = nullptr;
        ExitWindowEvent OnExitWindow = nullptr;

    public:

        virtual GFXRenderer* GetRenderer() = 0;

        virtual GFXBuffer_sp CreateBuffer(GFXBufferUsage usage, size_t bufferSize) = 0;
        virtual GFXCommandBuffer_sp CreateCommandBuffer() = 0;
        virtual GFXVertexLayoutDescription_sp CreateVertexLayoutDescription() = 0;
        virtual GFXGpuProgram_sp CreateGpuProgram(const std::unordered_map<gfx::GFXShaderStageFlags, array_list<char>>& codes) = 0;
        virtual GFXShaderPass_sp CreateShaderPass(
            const GFXShaderPassConfig& config,
            const GFXGpuProgram_sp& gpuProgram) = 0;

        virtual GFXDescriptorManager* GetDescriptorManager() = 0;

        virtual GFXDescriptorSetLayout_sp CreateDescriptorSetLayout(
            const GFXDescriptorSetLayoutInfo* layouts,
            size_t layoutCount) = 0;
        virtual GFXDescriptorSetLayout_sp CreateDescriptorSetLayout(
            std::initializer_list<GFXDescriptorSetLayoutInfo> layouts);

        virtual GFXGraphicsPipelineManager* GetGraphicsPipelineManager() const = 0;


        virtual GFXTexture_sp CreateTexture2DFromMemory(
            const uint8_t* imageData, size_t length,
            int width, int height,
            GFXTextureFormat format,
            const GFXSamplerConfig& samplerConfig
            ) = 0;

        virtual GFXTexture_sp CreateTextureCube(int32_t size) = 0;

        virtual GFXTexture_sp CreateRenderTarget(
            int32_t width, int32_t height, GFXTextureTargetType type,
            GFXTextureFormat format, const GFXSamplerConfig& samplerCfg) = 0;

        virtual GFXFrameBufferObject_sp CreateFrameBufferObject(
            const array_list<GFXTexture2DView_sp>& renderTargets,
            const GFXRenderPassLayout_sp& renderPassLayout) = 0;

        virtual GFXRenderPassLayout_sp CreateRenderPassLayout(const array_list<GFXTexture2DView*>& renderTargets) = 0;

        virtual array_list<GFXTextureFormat> GetSupportedDepthFormats() = 0;

        virtual intptr_t GetWindowHandle() = 0;

        virtual GFXViewport* GetViewport() = 0;

    protected:
        GFXApplication() = default;

    protected:
        GFXGlobalConfig m_config{};
    };

} // namespace gfx