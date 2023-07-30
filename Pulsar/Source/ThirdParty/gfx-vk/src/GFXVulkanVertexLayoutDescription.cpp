#include <gfx-vk/GFXVulkanVertexLayoutDescription.h>
#include <unordered_map>
#include <cassert>

namespace gfx
{
    VkVertexInputBindingDescription GFXVulkanVertexLayoutDescription::GetVkBindingDescription() const
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = this->BindingPoint;
        bindingDescription.stride = this->Stride;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    static VkFormat _GetVkFormat(GFXVertexInputDataFormat format)
    {
        static std::unordered_map<GFXVertexInputDataFormat, VkFormat> map
        {
            { GFXVertexInputDataFormat::R32G32B32_SFloat, VK_FORMAT_R32G32B32_SFLOAT },
            { GFXVertexInputDataFormat::R32G32B32A32_SFloat, VK_FORMAT_R32G32B32A32_SFLOAT },
            { GFXVertexInputDataFormat::R32G32_SFloat, VK_FORMAT_R32G32_SFLOAT }
        };
        auto it = map.find(format);
        assert(it != map.end());
        return it->second;
    }
    std::vector<VkVertexInputAttributeDescription> GFXVulkanVertexLayoutDescription::GetVkAttributeDescriptions() const
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        for (size_t i = 0; i < this->Attributes.size(); i++)
        {
            auto& item = this->Attributes[i];
            VkVertexInputAttributeDescription description;
            description.binding = this->BindingPoint;
            description.location = static_cast<uint32_t>(i);
            description.format = _GetVkFormat(item.Format);
            description.offset = item.Offset;
            attributeDescriptions.push_back(description);
        }

        return attributeDescriptions;
    }
}