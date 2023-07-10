#pragma once
#include <gfx/GFXVertexLayoutDescription.h>
#include "VulkanInclude.h"

namespace gfx
{
    class GFXVulkanVertexLayoutDescription : public GFXVertexLayoutDescription
    {
        using base = GFXVertexLayoutDescription;
    public:
        VkVertexInputBindingDescription GetVkBindingDescription();
        std::vector<VkVertexInputAttributeDescription> GetVkAttributeDescriptions();
    };
}