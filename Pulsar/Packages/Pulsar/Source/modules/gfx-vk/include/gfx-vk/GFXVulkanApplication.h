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
#include "GFXVulkanResourceRegistry.h"
#include "gfx/GFXTextureView.h"
#include <chrono>

namespace gfx
{
    class GFXVulkanTexture;

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

        virtual void Initialize() override;
        virtual void Terminate() override;
        virtual void WaitDeviceIdle() override;
        // Batches a staging->device copy into the frame-head transfer flush.
        // Used by backend resources (GFXVulkanBuffer::Update); not engine-facing.
        void RequestBufferUpload(GFXBuffer* dst, const void* data, size_t size, size_t dstOffset = 0);
        void RequestTextureUpload(GFXVulkanTexture* dst, const void* data, size_t size);

        virtual GFXApi GetApiType() const override { return GFXApi::Vulkan; }
        virtual const char* GetApiLevelName() const override { return "Vulkan 1.3"; }

        void TickRender(float deltaTime);

        virtual GFXGraphicsPipelineManager* GetGraphicsPipelineManager() const override
        {
            return m_graphicsPipelineManager;
        }

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
        // Timeline semaphore signaled by every frame submission on the graphics queue.
        const VkSemaphore& GetTimelineSemaphore() const { return m_timelineSemaphore; }
        uint64_t GetFrameCount() const override { return m_framecount; }
        uint64_t GetCompletedTimeline() const override;
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
        class GFXVulkanTransferQueue* GetTransferQueue() const
        {
            return m_uploadQueue;
        }

        // Device-level command buffer allocator for the graphics queue family.
        // Frame recording goes through the renderer; one-off sync paths (readback)
        // may allocate from it directly.
        class GFXVulkanCommandBufferPool* GetCommandBufferPool() const
        {
            return m_cmdPool;
        }

        // Single monotonic frame counter owned by the gfx layer. Incremented once per successful
        // frame (skipped/early-out frames do not advance it). All in-flight indices are derived
        // from this via (GetFrameCount() % kFrameSlotCount).
        uint32_t GetFrameInFlightIndex() const override { return static_cast<uint32_t>(m_framecount % kFrameSlotCount); }

        // GPU in-flight depth is 1 (frame N is submitted only after N-1 completed), but
        // two slots are needed because recording frame N overlaps execution of frame N-1.
        static constexpr int kFrameSlotCount = 2;
        uint32_t GetFrameSlotCount() const override { return kFrameSlotCount; }

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
        VkSemaphore m_timelineSemaphore = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;



        class GFXVulkanTransferQueue* m_uploadQueue = nullptr;
        class GFXVulkanCommandBufferPool* m_cmdPool = nullptr;

        // Per-slot command buffers, allocated once at init and reused in rotation
        // (reset happens in Begin(); the slot's previous submission completed two
        // frames ago, covered by the submit-time timeline wait).
        struct FrameCommandBuffers
        {
            std::unique_ptr<GFXVulkanCommandBuffer> render;
            std::unique_ptr<GFXVulkanCommandBuffer> transfer;
        };
        FrameCommandBuffers m_frameCmds[kFrameSlotCount];

        GFXGraphicsPipelineManager* m_graphicsPipelineManager = nullptr;

        array_list<const char*> m_extensions;
        size_t m_count = 0;

        uint64_t m_framecount = 0;

        std::vector<GFXTextureFormat> m_depthFormatCache;

    };


}
