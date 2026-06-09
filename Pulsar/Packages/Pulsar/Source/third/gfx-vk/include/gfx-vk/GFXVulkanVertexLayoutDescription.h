#pragma once
#include <gfx/GFXVertexLayoutDescription.h>
#include "VulkanInclude.h"

namespace gfx
{
    // Free helpers that convert the CPU-side vertex layout description into
    // Vulkan structures at pipeline-creation time.
    VkVertexInputBindingDescription GetVkVertexBindingDescription(const GFXVertexLayoutDescription& layout);
    array_list<VkVertexInputAttributeDescription> GetVkVertexAttributeDescriptions(const GFXVertexLayoutDescription& layout);
    array_list<VkVertexInputAttributeDescription> GetCombinedVkVertexAttributes(const array_list<GFXVertexLayoutDescription>& layouts);
}
