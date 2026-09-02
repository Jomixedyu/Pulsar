#pragma once
#include <vulkan/vulkan.h>
#include <optional>

namespace gfx::vk
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    class PhysicalDeviceHelper
    {
    public:
        static QueueFamilyIndices FindQueueFamilies(VkSurfaceKHR surface, VkPhysicalDevice device);
        static bool IsDeviceSuitable(VkSurfaceKHR surface, VkPhysicalDevice device);

    };
}