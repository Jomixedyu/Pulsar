#pragma once
#include <gfx/GFXVertexLayoutDescription.h>
#include "VulkanInclude.h"

namespace gfx
{
    class GFXVulkanVertexLayoutDescription : public GFXVertexLayoutDescription
    {
        using base = GFXVertexLayoutDescription;
    public:
        VkVertexInputBindingDescription GetVkBindingDescription() const;
        array_list<VkVertexInputAttributeDescription> GetVkAttributeDescriptions() const;

        static array_list<VkVertexInputAttributeDescription> GetCombinedAttributes(
            const array_list<GFXVertexLayoutDescription_sp>& attributes);
    };
    GFX_DECL_SPTR(GFXVulkanVertexLayoutDescription);
}