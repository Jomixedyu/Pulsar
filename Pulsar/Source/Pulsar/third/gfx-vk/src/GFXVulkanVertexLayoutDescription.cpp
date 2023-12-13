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
            { GFXVertexInputDataFormat::R32G32_SFloat, VK_FORMAT_R32G32_SFLOAT },
            { GFXVertexInputDataFormat::R8G8B8A8_UInt, VK_FORMAT_R8G8B8A8_UINT }
        };
        auto it = map.find(format);
        assert(it != map.end());
        return it->second;
    }

    array_list<VkVertexInputAttributeDescription> GFXVulkanVertexLayoutDescription::GetVkAttributeDescriptions() const
    {
        array_list<VkVertexInputAttributeDescription> attributeDescriptions;

        for (size_t i = 0; i < this->Attributes.size(); i++)
        {
            auto& item = this->Attributes[i];
            VkVertexInputAttributeDescription description;
            description.binding = this->BindingPoint;
            description.location = static_cast<uint32_t>(item.Location);
            description.format = _GetVkFormat(item.Format);
            description.offset = item.Offset;
            attributeDescriptions.push_back(description);
        }

        return attributeDescriptions;
    }

    array_list<VkVertexInputAttributeDescription> GFXVulkanVertexLayoutDescription::GetCombinedAttributes(
        const array_list<GFXVertexLayoutDescription_sp>& attributes)
    {
        array_list<VkVertexInputAttributeDescription> attributeDescriptions;

        for (auto& attrDesc : attributes)
        {
            for (size_t i = 0; i < attrDesc->Attributes.size(); i++)
            {
                auto& item = attrDesc->Attributes[i];
                VkVertexInputAttributeDescription description;
                description.binding = attrDesc->BindingPoint;
                description.location = static_cast<uint32_t>(item.Location);
                description.format = _GetVkFormat(item.Format);
                description.offset = item.Offset;
                attributeDescriptions.push_back(description);
            }
        }

        return attributeDescriptions;
    }

}