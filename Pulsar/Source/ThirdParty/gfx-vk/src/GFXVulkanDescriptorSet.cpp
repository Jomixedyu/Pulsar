#include <gfx-vk/GFXVulkanDescriptorSet.h>
#include <gfx-vk/GFXVulkanApplication.h>
#include <gfx-vk/GFXVulkanBuffer.h>
#include <gfx-vk/GFXVulkanTexture2D.h>
#include <gfx-vk/GFXVulkanDescriptorPool.h>
#include <cassert>
#include <stdexcept>

namespace gfx
{
    static std::unordered_map<VkDescriptorType, float> DefaultPoolSizes
    {
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER, 2 },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2 },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 / 8.0f },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1 / 2.0f },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1 / 8.0f },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 / 4.0f },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 / 8.0f },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 4 },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 / 8.0f },
        { VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1 / 8.0f },
    };


    static VkDescriptorType _GetDescriptorType(GFXDescriptorType type)
    {
        switch (type)
        {
        case gfx::GFXDescriptorType::ConstantBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
        case gfx::GFXDescriptorType::CombinedImageSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            break;
        default:
            assert(false);
            break;
        }
        return {};
    }

    GFXVulkanDescriptorSetLayout::GFXVulkanDescriptorSetLayout(
        GFXVulkanApplication* app,
        const std::vector<GFXDescriptorSetLayoutInfo>& layout
    )
        : base(layout), m_app(app)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (size_t i = 0; i < m_layout.size(); i++)
        {
            auto& layout = m_layout[i];
            VkDescriptorSetLayoutBinding binding{};
            binding.binding = layout.BindingPoint;
            binding.descriptorType = _GetDescriptorType(layout.Type);
            binding.descriptorCount = 1;
            binding.stageFlags = (uint32_t)layout.Stage;
            binding.pImmutableSamplers = nullptr;

            bindings.push_back(binding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(app->GetVkDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    GFXVulkanDescriptorSetLayout::~GFXVulkanDescriptorSetLayout()
    {
        if (m_descriptorSetLayout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(m_app->GetVkDevice(), m_descriptorSetLayout, nullptr);
            m_descriptorSetLayout = VK_NULL_HANDLE;
        }
    }


    void GFXVulkanDescriptor::SetConstantBuffer(size_t size, GFXBuffer* buffer)
    {
        auto vkBuffer = static_cast<GFXVulkanBuffer*>(buffer);

        BufferInfo.buffer = vkBuffer->GetVkBuffer();
        BufferInfo.offset = 0;
        BufferInfo.range = size;

        VkWriteDescriptorSet write;
        WriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        WriteInfo.dstSet = m_descriptorSet->GetVkDescriptorSet();
        WriteInfo.dstBinding = m_bindingPoint;
        WriteInfo.dstArrayElement = 0;
        WriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        WriteInfo.descriptorCount = 1;
        WriteInfo.pBufferInfo = &BufferInfo;

    }
    void GFXVulkanDescriptor::SetTextureSampler2D(GFXTexture2D* texture)
    {
        auto vkTex2d = static_cast<GFXVulkanTexture2D*>(texture);

        ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageInfo.imageView = vkTex2d->GetVkImageView();
        ImageInfo.sampler = vkTex2d->GetVkSampler();

        WriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        WriteInfo.dstSet = m_descriptorSet->GetVkDescriptorSet();
        WriteInfo.dstBinding = m_bindingPoint;
        WriteInfo.dstArrayElement = 0;
        WriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        WriteInfo.descriptorCount = 1;
        WriteInfo.pImageInfo = &ImageInfo;
    }


    GFXVulkanDescriptorSet::GFXVulkanDescriptorSet(GFXVulkanDescriptorPool* pool, GFXDescriptorSetLayout* layout)
        : m_pool(pool)
    {
        m_setlayout = static_cast<GFXVulkanDescriptorSetLayout*>(layout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pool->GetVkDescriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_setlayout->GetVkDescriptorSetLayout();

        auto result = vkAllocateDescriptorSets(pool->GetApplication()->GetVkDevice(), &allocInfo, &m_descriptorSet);
        assert(result == VK_SUCCESS);
    }

    GFXVulkanDescriptorSet::~GFXVulkanDescriptorSet()
    {
        m_descriptors.clear();

        vkFreeDescriptorSets(this->GetApplication()->GetVkDevice(), m_pool->GetVkDescriptorPool(), 1, &m_descriptorSet);
        m_descriptorSet = VK_NULL_HANDLE;
        m_pool->ReleaseDescriptorSet();
    }

    GFXDescriptor* GFXVulkanDescriptorSet::AddDescriptor(uint32_t bindingPoint)
    {
        auto descriptor = new GFXVulkanDescriptor{ this, bindingPoint };
        m_descriptors.push_back(std::unique_ptr<GFXVulkanDescriptor>{ descriptor });
        return descriptor;
    }

    void GFXVulkanDescriptorSet::Submit()
    {
        std::vector<VkWriteDescriptorSet> writeInfos;
        for (auto& descriptor : m_descriptors)
        {
            writeInfos.push_back(descriptor->WriteInfo);
        }
        vkUpdateDescriptorSets(m_pool->GetApplication()->GetVkDevice(), writeInfos.size(), writeInfos.data(), 0, nullptr);
    }

    GFXVulkanApplication* GFXVulkanDescriptorSet::GetApplication() const
    {
        return m_pool->GetApplication();
    }



}