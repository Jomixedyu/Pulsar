#include "GFXVulkanApplication.h"
#include <glfw/include/GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include "PhysicalDeviceHelper.h"
#include "BufferHelper.h"
#include "GFXVulkanCommandBuffer.h"
#include "GFXVulkanBuffer.h"
#include "GFXVulkanVertexLayoutDescription.h"
#include "GFXVulkanTexture2D.h"
#include "GFXVulkanDescriptorManager.h"
#include "GFXVulkanShaderModule.h"
#include "GFXVulkanShaderPass.h"
#include "GFXVulkanRenderer.h"
#include "GFXVulkanRenderPass.h"
#include "GFXVulkanViewport.h"
#include "GFXVulkanCommandBufferPool.h"
#include <set>
#include <cmath>
#include <array>
#include <algorithm>
#include <stdlib.h>

#undef max

namespace gfx
{
    GFXExtensions GFXVulkanApplication::GetExtensionNames()
    {
        if (m_extensions.empty())
        {
            uint32_t     glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            m_count = glfwExtensionCount;

            for (uint32_t i = 0; i < glfwExtensionCount; i++)
            {
                const size_t strlen = ::strlen(glfwExtensions[i]);
                const size_t bufferSize = strlen + 1;

                auto str = new char[bufferSize];
                ::strcpy(str, glfwExtensions[i]);

                m_extensions.push_back(str);
            }

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
        return reinterpret_cast<intptr_t>(m_window);
    }

    void GFXVulkanApplication::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<GFXVulkanApplication*>(glfwGetWindowUserPointer(window));
        app->m_framebufferResized = true;
    }

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
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
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
        dark_aqua, dark_cyan = 3,
        dark_red = 4,
        dark_purple = 5, dark_pink = 5, dark_magenta = 5,
        dark_yellow = 6,
        dark_white = 7,
        gray = 8,
        blue = 9,
        green = 10,
        aqua = 11, cyan = 11,
        red = 12,
        purple = 13, pink = 13, magenta = 13,
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
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
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
        appInfo.apiVersion = VK_API_VERSION_1_2;

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
        else {
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
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        vk::QueueFamilyIndices indices = vk::PhysicalDeviceHelper::FindQueueFamilies(m_surface, m_physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

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

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (m_config.EnableValid)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
            _PopulateDebugMessengerCreateInfo(debugCreateInfo);
            //createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
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



    void GFXVulkanApplication::Initialize()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_window = glfwCreateWindow(m_config.WindowWidth, m_config.WindowHeight, m_config.Title, nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);

        this->InitVkInstance();

        // setup debuger
        if (m_config.EnableValid)
        {
            VkDebugUtilsMessengerCreateInfoEXT createInfo{};
            _PopulateDebugMessengerCreateInfo(createInfo);

            //if (_CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) 
            //{
            //    throw std::runtime_error("failed to set up debug messenger!");
            //}
        }
        // create surface
        if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }

        this->InitPickPhysicalDevice();
        this->InitLogicalDevice();

        // command pool
        m_cmdPool = new GFXVulkanCommandBufferPool(this);
        // descriptor
        m_descriptorManager = new GFXVulkanDescriptorManager(this);
        // viewport
        m_viewport = new GFXVulkanViewport(this, m_window);
        // renderer
        m_renderer = new GFXVulkanRenderer(this);
    }

    void GFXVulkanApplication::ExecLoop()
    {
        m_startTime = std::chrono::high_resolution_clock::now();
        m_lastTime = std::chrono::high_resolution_clock::now();
        while (!m_isAppEnding)
        {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_lastTime).count();

            if (glfwWindowShouldClose(m_window))
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

            glfwPollEvents();

            if (OnPreRender)
            {
                OnPreRender(deltaTime);
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
        delete m_renderer;
        delete m_viewport;
        delete m_descriptorManager;
        //
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

        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    GFXBuffer* GFXVulkanApplication::CreateBuffer(GFXBufferUsage usage, size_t bufferSize)
    {
        return new GFXVulkanBuffer(this, usage, bufferSize);
    }
    std::shared_ptr<GFXCommandBuffer> gfx::GFXVulkanApplication::CreateCommandBuffer()
    {
        return std::shared_ptr<GFXCommandBuffer>(new GFXVulkanCommandBuffer(this));
    }
    std::shared_ptr<GFXVertexLayoutDescription> gfx::GFXVulkanApplication::CreateVertexLayoutDescription()
    {
        return std::shared_ptr<GFXVertexLayoutDescription>(new GFXVulkanVertexLayoutDescription());
    }

    std::shared_ptr<GFXTexture2D> gfx::GFXVulkanApplication::CreateTexture2DFromMemory(
        const uint8_t* data, int32_t length, const GFXSamplerConfig& samplerConfig, bool enableReadWrite, GFXTextureFormat format)
    {
        return GFXVulkanTexture2D::CreateFromMemory(this, data, length, enableReadWrite, format, samplerConfig);
    }

    std::shared_ptr<GFXFrameBufferObject> GFXVulkanApplication::CreateFrameBufferObject(
        const std::vector<GFXRenderTarget*>& renderTargets,
        const std::shared_ptr<GFXRenderPassLayout>& renderPassLayout)
    {
        std::vector<GFXVulkanRenderTarget*> rts;
        rts.reserve(renderTargets.size());

        std::transform(
            renderTargets.begin(), renderTargets.end(),
            std::back_insert_iterator<decltype(rts)>(rts),
            [](GFXRenderTarget* rt) { return static_cast<GFXVulkanRenderTarget*>(rt); });

        auto buf = new GFXVulkanFrameBufferObject(this, rts, std::static_pointer_cast<GFXVulkanRenderPass>(renderPassLayout));
        return std::shared_ptr<GFXFrameBufferObject>(buf);
    }

    std::shared_ptr<GFXShaderModule> GFXVulkanApplication::CreateShaderModule(const std::vector<uint8_t>& vert, const std::vector<uint8_t>& frag)
    {
        return std::shared_ptr<GFXShaderModule>(new GFXVulkanShaderModule(this, vert, frag));
    }

    std::shared_ptr<GFXShaderPass> GFXVulkanApplication::CreateGraphicsPipeline(
        const GFXShaderPassConfig& config,
        std::shared_ptr<GFXVertexLayoutDescription> vertexLayout,
        std::shared_ptr<GFXShaderModule> shaderModule,
        const std::shared_ptr<GFXDescriptorSetLayout>& descSetLayout,
        GFXRenderPassLayout* renderPass)
    {
        auto vkRenderPass = static_cast<GFXVulkanRenderPass*>(renderPass);
        auto vkPipeline = new GFXVulkanShaderPass(this, config, vertexLayout, shaderModule, descSetLayout, vkRenderPass);
        return std::shared_ptr<GFXShaderPass>(vkPipeline);
    }

    std::shared_ptr<GFXRenderPassLayout> GFXVulkanApplication::CreateRenderPassLayout(const std::vector<GFXRenderTarget*>& renderTargets)
    {
        std::vector<GFXVulkanRenderTarget*> rt;
        for (auto i : renderTargets)
        {
            rt.push_back(static_cast<GFXVulkanRenderTarget*>(i));
        }
        return std::shared_ptr<GFXRenderPassLayout>(new GFXVulkanRenderPass(this, rt));
    }

    std::shared_ptr<GFXRenderTarget> GFXVulkanApplication::CreateRenderTarget(
        int32_t width, int32_t height, GFXRenderTargetType type, GFXTextureFormat format, const GFXSamplerConfig& samplerCfg)
    {
        auto rt = new GFXVulkanRenderTarget(this, width, height, type, format, samplerCfg);
        return std::shared_ptr<GFXRenderTarget>(rt);
    }

    GFXDescriptorManager* GFXVulkanApplication::GetDescriptorManager()
    {
        return m_descriptorManager;
    }
}