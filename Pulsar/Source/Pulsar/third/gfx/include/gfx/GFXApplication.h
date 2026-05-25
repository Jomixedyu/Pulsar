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
#include "GFXSwapchain.h"
#include "GFXTextureView.h"
#include "GFXVertexLayoutDescription.h"
#include "GFXGlobalShaderManager.h"
#include "GFXSurface.h"
#include "GFXCommandList.h"
#include <functional>
#include <memory>

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
        virtual ~GFXApplication();

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
            m_shaderManager.Clear();
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

        virtual GFXRefCountPtr<GFXBuffer> CreateBuffer(const GFXBufferDesc& desc) = 0;
        virtual GFXCommandBuffer_sp CreateCommandBuffer() = 0;
        virtual GFXRefCountPtr<GFXVertexLayoutDescription> CreateVertexLayoutDescription() = 0;
        virtual GFXRefCountPtr<GFXGpuProgram> CreateGpuProgram(GFXGpuProgramStageFlags stage, const uint8_t* code, size_t length) = 0;

        virtual GFXDescriptorManager* GetDescriptorManager() = 0;

        virtual GFXRefCountPtr<GFXDescriptorSetLayout> CreateDescriptorSetLayout(
            const GFXDescriptorSetLayoutDesc* layouts,
            size_t layoutCount) = 0;
        virtual GFXRefCountPtr<GFXDescriptorSetLayout> CreateDescriptorSetLayout(
            std::initializer_list<GFXDescriptorSetLayoutDesc> layouts);

        virtual GFXGraphicsPipelineManager* GetGraphicsPipelineManager() const = 0;


        virtual GFXRefCountPtr<GFXTexture> CreateTexture2DFromMemory(
            const uint8_t* imageData, size_t length,
            int width, int height,
            GFXTextureFormat format,
            const GFXSamplerConfig& samplerConfig
            ) = 0;

        virtual GFXRefCountPtr<GFXTexture> CreateTextureCube(int32_t size) = 0;

        virtual GFXRefCountPtr<GFXTexture> CreateRenderTarget(
            int32_t width, int32_t height, GFXTextureTargetType type,
            GFXTextureFormat format, const GFXSamplerConfig& samplerCfg,
            uint32_t sampleCount = 1, bool isTransientAttachment = false) = 0;

        virtual GFXRefCountPtr<GFXFrameBufferObject> CreateFrameBufferObject(
            const array_list<GFXTexture2DView_sp>& renderTargets) = 0;

        virtual array_list<GFXTextureFormat> GetSupportedDepthFormats() = 0;

        virtual std::vector<uint8_t> ReadbackTexture(GFXTexture* texture, int32_t width, int32_t height) = 0;

        virtual intptr_t GetWindowHandle() = 0;
        virtual GFXSurface* GetWindow() = 0;

        virtual GFXSwapchain* GetViewport() = 0;

        GFXCommandList& GetCommandList() const { return *m_commandList; }
        GFXCommandList& GetImmediateCommandList() const { return *m_immediateCommandList; }
        GFXResourceManager* GetResourceManager() const { return m_resourceManager.get(); }

        GFXGlobalShaderManager& GetGlobalShaderManager() { return m_shaderManager; }

    protected:
        GFXApplication() = default;

    protected:
        GFXGlobalConfig m_config{};
        GFXGlobalShaderManager m_shaderManager;
        std::unique_ptr<GFXResourceManager> m_resourceManager;
        std::unique_ptr<GFXCommandList> m_commandList;
        std::unique_ptr<GFXCommandList> m_immediateCommandList;
    };

} // namespace gfx