#pragma once
#include <gfx/GFXApplication.h>

#include "VulkanInclude.h"

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/include/GLFW/glfw3.h>
#include <glfw/include/GLFW/glfw3native.h>

#include <chrono>
#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanViewport.h"
#include "GFXVulkanRenderPass.h"

namespace gfx
{
    class GFXVulkanApplication : public GFXApplication
    {
    public:
        GFXVulkanApplication(GFXGlobalConfig config)
        {
            m_config = config;
        }


    public:
        virtual void Initialize() override;
        virtual void ExecLoop() override;
        virtual void RequestStop() override;
        virtual void Terminate() override;

        void TickRender(float deltaTime);
        virtual GFXBuffer* CreateBuffer(GFXBufferUsage usage, size_t bufferSize) override;
        virtual std::shared_ptr<GFXCommandBuffer> CreateCommandBuffer() override;
        virtual std::shared_ptr<GFXVertexLayoutDescription> CreateVertexLayoutDescription() override;
        virtual std::shared_ptr<GFXShaderModule> CreateShaderModule(const std::vector<uint8_t>& vert, const std::vector<uint8_t>& frag) override;
        virtual std::shared_ptr<GFXShaderPass> CreateGraphicsPipeline(
            const GFXShaderPassConfig& config,
            std::shared_ptr<GFXVertexLayoutDescription> VertexLayout,
            std::shared_ptr<GFXShaderModule> ShaderModule,
            const std::shared_ptr<GFXDescriptorSetLayout>& descSetLayout,
            GFXRenderPassLayout* renderPass) override;

        virtual std::shared_ptr<GFXTexture2D> CreateTexture2DFromMemory(
            const uint8_t* data, int32_t length,
            const GFXSamplerConfig& samplerConfig,
            bool enableReadWrite = false, 
            GFXTextureFormat format = GFXTextureFormat::R8G8B8A8_SRGB) override;

        virtual GFXDescriptorManager* GetDescriptorManager() override;
        class GFXVulkanDescriptorManager* GetVulkanDescriptorManager() const { return m_descriptorManager; }
        virtual GFXExtensions GetExtensionNames() override;
        virtual intptr_t GetWindowHandle() override;

        GLFWwindow* GetWindow() const { return m_window; }
    public:
        const VkDevice& GetVkDevice() const { return m_device; }
        const VkPhysicalDevice& GetVkPhysicalDevice() const { return m_physicalDevice; }
        const VkInstance& GetVkInstance() const { return m_instance; }
        const VkSurfaceKHR& GetVkSurface() const { return m_surface; }
        const VkQueue& GetVkGraphicsQueue() const { return m_graphicsQueue; }
        const VkQueue& GetVkPresentQueue() const { return m_presentQueue; }
        //const VkCommandPool& GetVkCommandPool() const { return m_commandPool; }

        virtual GFXViewport* GetViewport() override { return m_viewport; }
        GFXVulkanViewport* GetVulkanViewport() { return m_viewport; }

        void SetRenderPipeline(GFXRenderPipeline* pipeline)
        {
            m_renderPipeline = pipeline;
        }
        GFXRenderPipeline* GetRenderPipeline() const
        {
            return m_renderPipeline;
        }
        class GFXVulkanCommandBufferPool* GetCommandBufferPool() const
        {
            return m_cmdPool;
        }
    protected:
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    private:
        void InitVkInstance();

        void InitPickPhysicalDevice();
        void InitLogicalDevice();

    public:

    protected:

        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        GLFWwindow* m_window = nullptr;
        bool m_framebufferResized = false;

        VkInstance m_instance = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
        //VkCommandPool m_commandPool = VK_NULL_HANDLE;

        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;

        GFXVulkanViewport* m_viewport;

        GFXRenderPipeline* m_renderPipeline = nullptr;

        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;

        class GFXVulkanDescriptorManager* m_descriptorManager = nullptr;
        class GFXVulkanRenderer* m_renderer = nullptr;

        class GFXVulkanCommandBufferPool* m_cmdPool = nullptr;

        array_list<char*> m_extensions;
        size_t m_count = 0;

        bool m_isAppEnding = false;

        uint32_t m_framecount = 0;

        std::chrono::steady_clock::time_point m_startTime;
        std::chrono::steady_clock::time_point m_lastTime;
    };

    class IGFXVulkanDevice
    {
        GFXVulkanApplication* m_vkapp;

        virtual void OnDeviceDestroy() = 0;
    };
}