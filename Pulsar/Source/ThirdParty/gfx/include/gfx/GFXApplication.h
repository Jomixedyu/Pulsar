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
#include <functional>
#include <memory>
#include <vector>

namespace gfx
{
    enum class GFXApi
    {
        Unknown,
        D3D12,
        Vulkan,
    };

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
        virtual int32_t GetApiType() const = 0;
        virtual const char* GetApiLevelName() const = 0;

        virtual void SetRenderPipeline(GFXRenderPipeline* pipeline) = 0;
        virtual GFXRenderPipeline* GetRenderPipeline() const = 0;

        LoopEvent OnPreRender = nullptr;
        LoopEvent OnPostRender = nullptr;
        ExitWindowEvent OnExitWindow = nullptr;
    public:
        virtual GFXBuffer* CreateBuffer(GFXBufferUsage usage, size_t bufferSize) = 0;
        virtual std::shared_ptr<GFXCommandBuffer> CreateCommandBuffer() = 0;
        virtual std::shared_ptr<GFXVertexLayoutDescription> CreateVertexLayoutDescription() = 0;
        virtual std::shared_ptr<GFXGpuProgram> CreateGpuProgram(const std::vector<uint8_t>& vert, const std::vector<uint8_t>& frag) = 0;
        virtual std::shared_ptr<GFXShaderPass> CreateShaderPass(
            const GFXShaderPassConfig& config,
            const std::shared_ptr<GFXGpuProgram>& gpuProgram,
            const std::shared_ptr<GFXDescriptorSetLayout>& descSetLayout,
            const std::shared_ptr<GFXVertexLayoutDescription>& vertexLayout) = 0;

        virtual GFXDescriptorManager* GetDescriptorManager() = 0;
        
        virtual std::shared_ptr<GFXDescriptorSetLayout> CreateDescriptorSetLayout(
            const std::vector<GFXDescriptorSetLayoutInfo>& layoutInfos) = 0;

        virtual GFXGraphicsPipelineManager* GetGraphicsPipelineManager() const = 0;

        virtual std::shared_ptr<GFXTexture2D> CreateTexture2DFromMemory(
            const uint8_t* data, int32_t length,
            const GFXSamplerConfig& samplerConfig,
            bool enableReadWrite = false, 
            GFXTextureFormat format = GFXTextureFormat::R8G8B8A8_SRGB) = 0;

        virtual std::shared_ptr<GFXRenderTarget> CreateRenderTarget(
            int32_t width, int32_t height, GFXRenderTargetType type,
            GFXTextureFormat format, const GFXSamplerConfig& samplerCfg) = 0;

        virtual std::shared_ptr<GFXFrameBufferObject> CreateFrameBufferObject(
            const std::vector<GFXRenderTarget*>& renderTargets,
            const std::shared_ptr<GFXRenderPassLayout>& renderPassLayout) = 0;

        virtual std::shared_ptr<GFXRenderPassLayout> CreateRenderPassLayout(const std::vector<GFXRenderTarget*>& renderTargets) = 0;

        virtual std::vector<GFXTextureFormat> GetSupportedDepthFormats() = 0;

        virtual intptr_t GetWindowHandle() = 0;

        virtual GFXViewport* GetViewport() = 0;
    protected:
        GFXApplication() {}
    protected:
        GFXGlobalConfig m_config{};
    };

}