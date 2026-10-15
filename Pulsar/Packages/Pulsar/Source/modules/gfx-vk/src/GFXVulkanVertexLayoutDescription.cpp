#include <gfx-vk/GFXVulkanVertexLayoutDescription.h>
#include <unordered_map>
#include <cassert>

namespace gfx
{
    VkVertexInputBindingDescription GetVkVertexBindingDescription(const GFXVertexLayoutDescription& layout)
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = layout.BindingPoint;
        bindingDescription.stride = layout.Stride;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    static VkFormat _GetVkFormat(GFXVertexInputDataFormat format)
    {
        static std::unordered_map<GFXVertexInputDataFormat, VkFormat> map
        {
            { GFXVertexInputDataFormat::R32G32B32_SFloat, VK_FORMAT_R32G32B32_SFLOAT },
            { GFXVertexInputDataFormat::R32G32B32A32_SFloat, VK_FORMAT_R32G32B32A32_SFLOAT },
            { GFXVertexInputDataFormat::R32G32_SFloat, VK_FORMAT_R32G32_SFLOAT },
            { GFXVertexInputDataFormat::R8G8B8A8_UInt, VK_FORMAT_R8G8B8A8_UINT },
            { GFXVertexInputDataFormat::R8G8B8A8_UNorm, VK_FORMAT_R8G8B8A8_UNORM }
        };
        auto it = map.find(format);
        assert(it != map.end());
        return it->second;
    }

    array_list<VkVertexInputAttributeDescription> GetVkVertexAttributeDescriptions(const GFXVertexLayoutDescription& layout)
    {
        array_list<VkVertexInputAttributeDescription> attributeDescriptions;

        for (size_t i = 0; i < layout.Attributes.size(); i++)
        {
            auto& item = layout.Attributes[i];
            VkVertexInputAttributeDescription description;
            description.binding = layout.BindingPoint;
            description.location = static_cast<uint32_t>(item.Location);
            description.format = _GetVkFormat(item.Format);
            description.offset = item.Offset;
            attributeDescriptions.push_back(description);
        }

        return attributeDescriptions;
    }

    array_list<VkVertexInputAttributeDescription> GetCombinedVkVertexAttributes(
        const array_list<GFXVertexLayoutDescription>& layouts)
    {
        array_list<VkVertexInputAttributeDescription> attributeDescriptions;

        for (auto& layout : layouts)
        {
            for (size_t i = 0; i < layout.Attributes.size(); i++)
            {
                auto& item = layout.Attributes[i];
                VkVertexInputAttributeDescription description;
                description.binding = layout.BindingPoint;
                description.location = static_cast<uint32_t>(item.Location);
                description.format = _GetVkFormat(item.Format);
                description.offset = item.Offset;
                attributeDescriptions.push_back(description);
            }
        }

        return attributeDescriptions;
    }

}
