#include "GFXVulkanApplication.h"
#include "BufferHelper.h"
#include "ImageHelper.h"
#include "GFXSurfaceSDL2.h"
#include "GFXVulkanBuffer.h"
#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanCommandBufferPool.h"
#include "GFXVulkanDescriptorManager.h"
#include "GFXVulkanGpuProgram.h"
#include "GFXVulkanGraphicsPipeline.h"
#include "GFXVulkanGraphicsPipelineManager.h"
#include "GFXVulkanRenderer.h"
#include "GFXVulkanSwapchain.h"
#include "GFXVulkanTexture.h"
#include "GFXVulkanVertexLayoutDescription.h"
#include "PhysicalDeviceHelper.h"

#include <SDL_vulkan.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <set>
#include <stdexcept>

#undef max

namespace gfx
{
    GFXExtensions GFXVulkanApplication::GetExtensionNames()
    {
        if (m_extensions.empty())
        {
            uint32_t extensionCount = 0;
            SDL_Vulkan_GetInstanceExtensions((SDL_Window*)m_window->GetUserPoint(), &extensionCount, nullptr);
            m_extensions.resize(extensionCount);
            SDL_Vulkan_GetInstanceExtensions((SDL_Window*)m_window->GetUserPoint(), &extensionCount, m_extensions.data());

            // const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            m_count = extensionCount;

            // for (uint32_t i = 0; i < extensionCount; i++)
            // {
            //     const size_t strlen = ::strlen(glfwExtensions[i]);
            //     const size_t bufferSize = strlen + 1;
            //
            //     auto str = new char[bufferSize];
            //     ::strcpy(str, glfwExtensions[i]);
            //
            //     m_extensions.push_back(str);
            // }

            if (GetConfig().EnableValid)
            {
                const size_t bufferSize = ::strlen(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) + 1;

                auto str = new char[bufferSize];
                ::strcpy(str, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

                m_extensions.push_back(str);
                ++m_count;
            }
        }

        return GFXExtensions(const_cast<const char* const*>(m_extensions.data()), m_count);
    }

    intptr_t GFXVulkanApplication::GetWindowHandle()
    {
        return m_window->GetNativeHandle();
    }

    // void GFXVulkanApplication::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    // {
    //     auto app = reinterpret_cast<GFXVulkanApplication*>(glfwGetWindowUserPointer(window));
    //     app->m_framebufferResized = true;
    // }

    static const std::vector<const char*> validationLayers =
        {
            "VK_LAYER_KHRONOS_validation"
        };

    static bool _CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    enum class ConsoleColor
    {
        black = 0,
        dark_blue = 1,
        dark_green = 2,
        dark_aqua,
        dark_cyan = 3,
        dark_red = 4,
        dark_purple = 5,
        dark_pink = 5,
        dark_magenta = 5,
        dark_yellow = 6,
        dark_white = 7,
        gray = 8,
        blue = 9,
        green = 10,
        aqua = 11,
        cyan = 11,
        red = 12,
        purple = 13,
        pink = 13,
        magenta = 13,
        yellow = 14,
        white = 15
    };
    static void _SetColor(ConsoleColor foreground, ConsoleColor background)
    {
        WORD consoleColor;
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;

        if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
        {
            consoleColor = ((int)foreground + ((int)background * 16));
            SetConsoleTextAttribute(hStdOut, consoleColor);
        }
    }
    static VKAPI_ATTR VkBool32 VKAPI_CALL _VkDebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            _SetColor(ConsoleColor::red, ConsoleColor::black);
        }
        else if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            _SetColor(ConsoleColor::yellow, ConsoleColor::black);
        }
        else if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            _SetColor(ConsoleColor::blue, ConsoleColor::black);
        }
        else if (messageSeverity & VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            _SetColor(ConsoleColor::white, ConsoleColor::black);
        }
        else
        {
            _SetColor(ConsoleColor::gray, ConsoleColor::black);
        }

        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
    static void _PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        //| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        createInfo.pfnUserCallback = _VkDebugCallback;
    }
    static VkResult _CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void GFXVulkanApplication::InitVkInstance()
    {
        if (m_config.EnableValid && !_CheckValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = m_config.ProgramName;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = m_config.ProgramName;
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_4;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = GetExtensionNames();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.GetCount());
        createInfo.ppEnabledExtensionNames = extensions.GetExtensionNames();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (m_config.EnableValid)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            _PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void GFXVulkanApplication::InitPickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

        for (const auto& device : devices)
        {
            if (vk::PhysicalDeviceHelper::IsDeviceSuitable(m_surface, device))
            {
                m_physicalDevice = device;
                break;
            }
        }

        if (m_physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void GFXVulkanApplication::InitLogicalDevice()
    {
        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,

        };

        vk::QueueFamilyIndices indices = vk::PhysicalDeviceHelper::FindQueueFamilies(m_surface, m_physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkPhysicalDeviceVulkan13Features features13{};
        features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        features13.dynamicRendering = VK_TRUE;
        features13.synchronization2 = VK_TRUE;

        VkPhysicalDeviceFeatures2 features2{};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features2.features = deviceFeatures;
        features2.pNext = &features13;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = nullptr;
        createInfo.pNext = &features2;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (m_config.EnableValid)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
            _PopulateDebugMessengerCreateInfo(debugCreateInfo);
            // createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
    }

    static bool _HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    GFXRenderer* GFXVulkanApplication::GetRenderer()
    {
        return m_renderer;
    }
    void GFXVulkanApplication::Initialize()
    {
        // glfwInit();
        // glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // m_window = glfwCreateWindow(m_config.WindowWidth, m_config.WindowHeight, m_config.Title, nullptr, nullptr);
        // glfwSetWindowUserPointer(m_window, this);
        // glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);


        auto win = new GFXSurfaceSDL2;
        m_window = win;

        win->Initialize();
        win->CreateMainWindow(m_config.Title, m_config.WindowWidth, m_config.WindowHeight);

        this->InitVkInstance();

        // setup debuger
        // if (m_config.EnableValid)
        // {
        //     VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        //     _PopulateDebugMessengerCreateInfo(createInfo);
        //
        //     // if (_CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
        //     //{
        //     //     throw std::runtime_error("failed to set up debug messenger!");
        //     // }
        // }


        if (win->CreateVulkanSurface(m_instance, &m_surface) == 0)
        {
            throw std::runtime_error("failed to create window surface!");
        }


        // create surface
        // if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
        // {
        //     throw std::runtime_error("failed to create window surface!");
        // }

        this->InitPickPhysicalDevice();
        this->InitLogicalDevice();

        m_cmdPool = new GFXVulkanCommandBufferPool(this);

        m_descriptorManager = new GFXVulkanDescriptorManager(this);

        m_viewport = new GFXVulkanSwapchain(this, m_window);

        m_renderer = new GFXVulkanRenderer(this);

        m_graphicsPipelineManager = new GFXVulkanGraphicsPipelineManager(this);
    }

    void GFXVulkanApplication::ExecLoop()
    {
        m_startTime = std::chrono::high_resolution_clock::now();
        m_lastTime = std::chrono::high_resolution_clock::now();
        while (!m_isAppEnding)
        {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_lastTime).count();

            // bool recivedClose = glfwWindowShouldClose(m_window);
            bool recivedClose = m_window->WantToClose();
            if (recivedClose)
            {
                if (OnExitWindow)
                {
                    if (OnExitWindow())
                    {
                        m_isAppEnding = true;
                    }
                }
                else
                {
                    m_isAppEnding = true;
                }
            }
            m_window->PollEvent();
            // glfwPollEvents();

            if (OnPreRender)
            {
                OnPreRender(deltaTime);
            }
            if (m_isAppEnding)
            {
                break;
            }
            TickRender(deltaTime);
            if (OnPostRender)
            {
                OnPostRender(deltaTime);
            }

            m_lastTime = currentTime;
            ++m_framecount;
        }
        vkDeviceWaitIdle(m_device);
    }

    void GFXVulkanApplication::TickRender(float deltaTime)
    {
        m_renderer->Render(deltaTime);
    }

    void GFXVulkanApplication::RequestStop()
    {
        m_isAppEnding = true;
    }

    void GFXVulkanApplication::Terminate()
    {
        base::Terminate();

        delete m_renderer;
        delete m_viewport;
        delete m_graphicsPipelineManager;
        delete m_descriptorManager;
        delete m_cmdPool;

        vkDestroyDevice(m_device, nullptr);

        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

        if (m_config.EnableValid)
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr && m_debugMessenger != VK_NULL_HANDLE)
            {
                func(m_instance, m_debugMessenger, nullptr);
            }
        }

        vkDestroyInstance(m_instance, nullptr);

        // glfwDestroyWindow(m_window);
        // glfwTerminate();

        m_window->DestroySurface();
        delete m_window;
        m_window = nullptr;
    }

    GFXBuffer_sp GFXVulkanApplication::CreateBuffer(const GFXBufferDesc& desc)
    {
        return gfxmksptr(new GFXVulkanBuffer(this, desc));
    }
    GFXCommandBuffer_sp gfx::GFXVulkanApplication::CreateCommandBuffer()
    {
        return gfxmksptr(new GFXVulkanCommandBuffer(this));
    }
    GFXVertexLayoutDescription_sp gfx::GFXVulkanApplication::CreateVertexLayoutDescription()
    {
        return gfxmksptr(new GFXVulkanVertexLayoutDescription());
    }


    std::shared_ptr<GFXTexture> gfx::GFXVulkanApplication::CreateTexture2DFromMemory(
        const uint8_t* imageData, size_t length, int width, int height, GFXTextureFormat format, const GFXSamplerConfig& samplerConfig)
    {
        GFXTextureCreateDesc info{};
        info.ImageData = imageData;
        info.DataLength = length;
        info.Width = width;
        info.Height = height;
        info.Depth = 1;
        info.Format = format;
        info.SamplerCfg = samplerConfig;
        info.DataType = GFXTextureDataType::Texture2D;

        return gfxmksptr(new GFXVulkanTexture(this, info));
    }

    std::shared_ptr<GFXFrameBufferObject> GFXVulkanApplication::CreateFrameBufferObject(
        const std::vector<GFXTexture2DView_sp>& renderTargets)
    {
        auto buf = new GFXVulkanFrameBufferObject(this, renderTargets);
        return gfxmksptr(buf);
    }

    GFXGpuProgram_sp GFXVulkanApplication::CreateGpuProgram(GFXGpuProgramStageFlags stage, const uint8_t* code, size_t length)
    {
        return gfxmksptr(new GFXVulkanGpuProgram(this, stage, code, length));
    }


    GFXTexture_sp GFXVulkanApplication::CreateTextureCube(int32_t size)
    {
        GFXTextureCreateDesc info{};
        info.Width = size;
        info.Height = size;
        info.Depth = 1;
        info.ArrayLayers = 6;
        info.Format = GFXTextureFormat::R16G16B16A16_SFloat;
        info.TargetType = GFXTextureTargetType::ColorTarget;
        info.DataType = GFXTextureDataType::TextureCube;
        return gfxmksptr(new GFXVulkanTexture(this, info));
    }

    GFXTexture_sp GFXVulkanApplication::CreateRenderTarget(
        int32_t width, int32_t height, GFXTextureTargetType type, GFXTextureFormat format, const GFXSamplerConfig& samplerCfg,
        uint32_t sampleCount, bool isTransientAttachment)
    {
        GFXTextureCreateDesc info{};
        info.Width = width;
        info.Height = height;
        info.Depth = 1;
        info.Format = format;
        info.TargetType = type;
        info.DataType = GFXTextureDataType::Texture2D;
        info.SampleCount = sampleCount;
        info.IsTransientAttachment = isTransientAttachment;

        auto rt = new GFXVulkanTexture(this, info);
        return gfxmksptr(rt);
    }

    GFXDescriptorManager* GFXVulkanApplication::GetDescriptorManager()
    {
        return m_descriptorManager;
    }

    GFXDescriptorSetLayout_sp GFXVulkanApplication::CreateDescriptorSetLayout(
        const GFXDescriptorSetLayoutDesc* layouts,
        size_t layoutCount)
    {
        return gfxmksptr(new GFXVulkanDescriptorSetLayout(this, layouts, layoutCount));
    }

    array_list<GFXTextureFormat> GFXVulkanApplication::GetSupportedDepthFormats()
    {
        if (m_depthFormatCache.empty())
        {
            for (auto& format : BufferHelper::FindDepthFormats(this))
            {
                m_depthFormatCache.push_back(BufferHelper::GetTextureFormat(format));
            }
        }
        return m_depthFormatCache;
    }

    std::vector<uint8_t> GFXVulkanApplication::ReadbackTexture(GFXTexture* texture, int32_t width, int32_t height)
    {
        std::vector<uint8_t> result;

        auto* vkTex = dynamic_cast<GFXVulkanTexture*>(texture);
        if (!vkTex)
        {
            return result;
        }

        VkImage srcImage = vkTex->GetVkImage();
        VkFormat srcFormat = vkTex->GetVkImageFormat();
        VkImageLayout currentLayout = vkTex->GetVkImageLayout();

        // Determine the bytes per pixel for the source format
        // We always convert to R8G8B8A8 on readback for PNG compatibility
        const VkFormat readbackFormat = VK_FORMAT_R8G8B8A8_UNORM;
        const size_t pixelSize = 4; // RGBA8
        const VkDeviceSize bufferSize = VkDeviceSize(width) * height * pixelSize;

        // Create the readback buffer (host visible, coherent)
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        BufferHelper::CreateBuffer(this, bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        // Create a command buffer for the copy
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_cmdPool->GetVkCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        // Transition source image to transfer src layout
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = currentLayout;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = srcImage;
        barrier.subresourceRange.aspectMask = vkTex->GetVkAspectFlags();
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = BufferHelper::GetAccessMaskForLayout(currentLayout);
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            BufferHelper::GetStageFlagsForLayout(currentLayout),
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        // If the source format needs conversion, use vkCmdBlitImage with a intermediate image
        // For simplicity, we use vkCmdCopyImageToBuffer for same-format, or blit for conversion
        bool needsConversion = (srcFormat != readbackFormat);

        if (needsConversion)
        {
            // Create a temporary R8G8B8A8_UNORM image as blit destination
            VkImage blitImage;
            VkDeviceMemory blitImageMemory;
            VkImageCreateInfo blitImageInfo{};
            blitImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            blitImageInfo.imageType = VK_IMAGE_TYPE_2D;
            blitImageInfo.extent.width = width;
            blitImageInfo.extent.height = height;
            blitImageInfo.extent.depth = 1;
            blitImageInfo.mipLevels = 1;
            blitImageInfo.arrayLayers = 1;
            blitImageInfo.format = readbackFormat;
            blitImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            blitImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            blitImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            blitImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            blitImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            ImageHelper::CreateImage(this, &blitImageInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, blitImage, blitImageMemory);

            // Transition blit destination to transfer dst
            VkImageMemoryBarrier blitDstBarrier{};
            blitDstBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            blitDstBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            blitDstBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            blitDstBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            blitDstBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            blitDstBarrier.image = blitImage;
            blitDstBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blitDstBarrier.subresourceRange.baseMipLevel = 0;
            blitDstBarrier.subresourceRange.levelCount = 1;
            blitDstBarrier.subresourceRange.baseArrayLayer = 0;
            blitDstBarrier.subresourceRange.layerCount = 1;
            blitDstBarrier.srcAccessMask = 0;
            blitDstBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &blitDstBarrier);

            // Blit (with format conversion)
            VkOffset3D blitSize{ width, height, 1 };
            VkImageBlit blitRegion{};
            blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blitRegion.srcSubresource.layerCount = 1;
            blitRegion.srcOffsets[0] = { 0, 0, 0 };
            blitRegion.srcOffsets[1] = blitSize;
            blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blitRegion.dstSubresource.layerCount = 1;
            blitRegion.dstOffsets[0] = { 0, 0, 0 };
            blitRegion.dstOffsets[1] = blitSize;

            vkCmdBlitImage(commandBuffer,
                srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                blitImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blitRegion,
                VK_FILTER_NEAREST);

            // Transition blit image to transfer src for copy to buffer
            VkImageMemoryBarrier blitSrcBarrier{};
            blitSrcBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            blitSrcBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            blitSrcBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            blitSrcBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            blitSrcBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            blitSrcBarrier.image = blitImage;
            blitSrcBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blitSrcBarrier.subresourceRange.baseMipLevel = 0;
            blitSrcBarrier.subresourceRange.levelCount = 1;
            blitSrcBarrier.subresourceRange.baseArrayLayer = 0;
            blitSrcBarrier.subresourceRange.layerCount = 1;
            blitSrcBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            blitSrcBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &blitSrcBarrier);

            // Copy blit image to buffer
            VkBufferImageCopy copyRegion{};
            copyRegion.bufferOffset = 0;
            copyRegion.bufferRowLength = 0;
            copyRegion.bufferImageHeight = 0;
            copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.imageSubresource.mipLevel = 0;
            copyRegion.imageSubresource.baseArrayLayer = 0;
            copyRegion.imageSubresource.layerCount = 1;
            copyRegion.imageOffset = { 0, 0, 0 };
            copyRegion.imageExtent = { (uint32_t)width, (uint32_t)height, 1 };

            vkCmdCopyImageToBuffer(commandBuffer,
                blitImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                stagingBuffer, 1, &copyRegion);

            // End command buffer and submit
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(m_graphicsQueue);

            // Read back from staging buffer
            void* data;
            vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
            result.resize(bufferSize);
            memcpy(result.data(), data, bufferSize);
            vkUnmapMemory(m_device, stagingBufferMemory);

            // Cleanup temporary blit image
            vkDestroyImage(m_device, blitImage, nullptr);
            vkFreeMemory(m_device, blitImageMemory, nullptr);
        }
        else
        {
            // Direct copy from image to buffer (same format)
            VkBufferImageCopy copyRegion{};
            copyRegion.bufferOffset = 0;
            copyRegion.bufferRowLength = 0;
            copyRegion.bufferImageHeight = 0;
            copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.imageSubresource.mipLevel = 0;
            copyRegion.imageSubresource.baseArrayLayer = 0;
            copyRegion.imageSubresource.layerCount = 1;
            copyRegion.imageOffset = { 0, 0, 0 };
            copyRegion.imageExtent = { (uint32_t)width, (uint32_t)height, 1 };

            vkCmdCopyImageToBuffer(commandBuffer,
                srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                stagingBuffer, 1, &copyRegion);

            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(m_graphicsQueue);

            // Read back from staging buffer
            void* data;
            vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
            result.resize(bufferSize);
            memcpy(result.data(), data, bufferSize);
            vkUnmapMemory(m_device, stagingBufferMemory);
        }

        // Restore the source image layout
        {
            VkCommandBuffer restoreCmdBuf;
            vkAllocateCommandBuffers(m_device, &allocInfo, &restoreCmdBuf);
            vkBeginCommandBuffer(restoreCmdBuf, &beginInfo);

            VkImageMemoryBarrier restoreBarrier{};
            restoreBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            restoreBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            restoreBarrier.newLayout = vkTex->GetVkTargetFinalLayout();
            restoreBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            restoreBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            restoreBarrier.image = srcImage;
            restoreBarrier.subresourceRange.aspectMask = vkTex->GetVkAspectFlags();
            restoreBarrier.subresourceRange.baseMipLevel = 0;
            restoreBarrier.subresourceRange.levelCount = 1;
            restoreBarrier.subresourceRange.baseArrayLayer = 0;
            restoreBarrier.subresourceRange.layerCount = 1;
            restoreBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            restoreBarrier.dstAccessMask = BufferHelper::GetAccessMaskForLayout(vkTex->GetVkTargetFinalLayout());

            vkCmdPipelineBarrier(restoreCmdBuf,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                BufferHelper::GetStageFlagsForLayout(vkTex->GetVkTargetFinalLayout()),
                0,
                0, nullptr,
                0, nullptr,
                1, &restoreBarrier);

            vkEndCommandBuffer(restoreCmdBuf);

            VkSubmitInfo restoreSubmit{};
            restoreSubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            restoreSubmit.commandBufferCount = 1;
            restoreSubmit.pCommandBuffers = &restoreCmdBuf;
            vkQueueSubmit(m_graphicsQueue, 1, &restoreSubmit, VK_NULL_HANDLE);
            vkQueueWaitIdle(m_graphicsQueue);

            vkFreeCommandBuffers(m_device, m_cmdPool->GetVkCommandPool(), 1, &restoreCmdBuf);
        }

        // Cleanup
        vkFreeCommandBuffers(m_device, m_cmdPool->GetVkCommandPool(), 1, &commandBuffer);
        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        vkFreeMemory(m_device, stagingBufferMemory, nullptr);

        // Update the tracked layout
        vkTex->SetImageLayout(vkTex->GetVkTargetFinalLayout());

        // Handle B8G8R8A8 -> R8G8B8A8 swizzle if needed
        if (srcFormat == VK_FORMAT_B8G8R8A8_UNORM || srcFormat == VK_FORMAT_B8G8R8A8_SRGB)
        {
            for (size_t i = 0; i < result.size(); i += 4)
            {
                std::swap(result[i + 0], result[i + 2]); // B <-> R
            }
        }

        return result;
    }
} // namespace gfx
