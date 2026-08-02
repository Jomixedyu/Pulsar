#pragma once
#include <gfx/GFXApplication.h>

#include <unordered_map>
#include <string>
#include <mutex>

#include <gfx/GFXSurface.h>
#include "VulkanInclude.h"
// #define GLFW_INCLUDE_VULKAN
// #define GLFW_EXPOSE_NATIVE_WIN32
// #include <glfw/include/GLFW/glfw3.h>
// #include <glfw/include/GLFW/glfw3native.h>

#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanSwapchain.h"
#include "gfx/GFXTextureView.h"
#include <chrono>

namespace gfx
{

    class GFXVulkanApplication : public GFXApplication
    {
    public:
        using base = GFXApplication;

        explicit GFXVulkanApplication(GFXGlobalConfig config)
        {
            m_config = config;
        }
        ~GFXVulkanApplication() override = default;

    public:

        virtual GFXRenderer* GetRenderer() override;

        virtual void Initialize() override;
        virtual void Terminate() override;
        virtual void WaitDeviceIdle() override;
        virtual void RequestBufferUpload(GFXBuffer* dst, const void* data, size_t size, size_t dstOffset = 0) override;

        virtual GFXApi GetApiType() const override { return GFXApi::Vulkan; }
        virtual const char* GetApiLevelName() const override { return "Vulkan 1.3"; }

        void TickRender(float deltaTime);
        virtual GFXBuffer_sp CreateBuffer(const GFXBufferDesc& desc) override;
        virtual GFXCommandBuffer_sp CreateCommandBuffer() override;
        virtual GFXGpuProgram_sp CreateGpuProgram(GFXGpuProgramStageFlags stage, const uint8_t* code, size_t length) override;


        virtual GFXGraphicsPipelineManager* GetGraphicsPipelineManager() const override
        {
            return m_graphicsPipelineManager;
        }

        virtual GFXTexture_sp CreateTexture2DFromMemory(
            const uint8_t* imageData, size_t length,
            int width, int height,
            GFXTextureFormat format,
            const GFXSamplerConfig& samplerConfig
            ) override;


        virtual GFXFrameBufferObject_sp CreateFrameBufferObject(
            const array_list<GFXTexture2DView_sp>& renderTargets) override;

        virtual GFXTexture_sp CreateTextureCube(int32_t size) override;

        virtual GFXSampler_sp CreateSampler(const GFXSamplerConfig& config) override;

        virtual GFXTexture_sp CreateRenderTarget(
            int32_t width, int32_t height, GFXTextureTargetType type,
            GFXTextureFormat format, const GFXSamplerConfig& samplerCfg,
            uint32_t sampleCount = 1, bool isTransientAttachment = false) override;

        virtual GFXDescriptorSetLayout_sp GetOrCreateDescriptorSetLayout(
            const GFXDescriptorLayoutDesc* layouts,
            size_t layoutCount) override;

        virtual array_list<GFXTextureFormat> GetSupportedDepthFormats() override;

        virtual std::vector<uint8_t> ReadbackTexture(GFXTexture* texture, int32_t width, int32_t height) override;

        virtual GFXExtensions GetExtensionNames() override;
        virtual intptr_t GetWindowHandle() override;
        virtual GFXSurface* GetWindow() override { return m_window; }
    public:
        const VkDevice& GetVkDevice() const { return m_device; }
        const VkPhysicalDevice& GetVkPhysicalDevice() const { return m_physicalDevice; }
        const VkInstance& GetVkInstance() const { return m_instance; }
        const VkSurfaceKHR& GetVkSurface() const { return m_surface; }
        const VkQueue& GetVkGraphicsQueue() const { return m_graphicsQueue; }
        const VkQueue& GetVkPresentQueue() const { return m_presentQueue; }
        //const VkCommandPool& GetVkCommandPool() const { return m_commandPool; }

        virtual GFXSwapchain* GetViewport() override { return m_viewport; }
        GFXVulkanSwapchain* GetVulkanViewport() { return m_viewport; }

        virtual void SetRenderPipeline(GFXRenderPipeline* pipeline) override
        {
            m_renderPipeline = pipeline;
        }
        virtual GFXRenderPipeline* GetRenderPipeline() const override
        {
            return m_renderPipeline;
        }
        class GFXVulkanCommandBufferPool* GetCommandBufferPool() const
        {
            return m_cmdPool;
        }

        class GFXVulkanBufferUploadQueue* GetBufferUploadQueue() const
        {
            return m_uploadQueue;
        }

        // Single monotonic frame counter owned by the gfx layer. Incremented once per successful
        // frame (skipped/early-out frames do not advance it). All in-flight indices are derived
        // from this via (GetFrameCount() % MAX_FRAMES_IN_FLIGHT).
        uint64_t GetFrameCount() const { return m_framecount; }
        uint32_t GetFrameInFlightIndex() const { return static_cast<uint32_t>(m_framecount % MAX_FRAMES_IN_FLIGHT); }

        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    private:
        void InitVkInstance();

        void InitPickPhysicalDevice();
        void InitLogicalDevice();

    public:

    protected:

        GFXSurface* m_window = nullptr;
        // bool m_framebufferResized = false;

        VkInstance m_instance = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
        //VkCommandPool m_commandPool = VK_NULL_HANDLE;

        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;

        GFXVulkanSwapchain* m_viewport = nullptr;

        GFXRenderPipeline* m_renderPipeline = nullptr;

        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;

        class GFXVulkanRenderer* m_renderer = nullptr;

        class GFXVulkanCommandBufferPool* m_cmdPool = nullptr;

        class GFXVulkanBufferUploadQueue* m_uploadQueue = nullptr;

        GFXGraphicsPipelineManager* m_graphicsPipelineManager = nullptr;

        array_list<const char*> m_extensions;
        size_t m_count = 0;

        uint64_t m_framecount = 0;

        std::vector<GFXTextureFormat> m_depthFormatCache;

        // Content-addressed descriptor set layout cache: identical binding content -> one shared object.
        std::unordered_map<std::string, GFXDescriptorSetLayout_sp> m_layoutCache;
        std::mutex m_layoutCacheMutex;
    };


}