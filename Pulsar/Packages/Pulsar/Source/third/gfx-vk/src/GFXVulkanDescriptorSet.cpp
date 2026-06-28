#include <cassert>
#include <gfx-vk/GFXVulkanApplication.h>
#include <gfx-vk/GFXVulkanBuffer.h>
#include <gfx-vk/GFXVulkanDescriptorSet.h>
#include <gfx-vk/GFXVulkanTexture.h>
#include <gfx-vk/GFXVulkanTextureView.h>
#include <stdexcept>

namespace gfx
{
    static VkDescriptorType _GetDescriptorType(GFXDescriptorType type)
    {
        switch (type)
        {
        case gfx::GFXDescriptorType::ConstantBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
        case gfx::GFXDescriptorType::ConstantBufferDynamic:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            break;
        case gfx::GFXDescriptorType::CombinedImageSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            break;
        case gfx::GFXDescriptorType::Texture2D:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case gfx::GFXDescriptorType::StructuredBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        default:
            assert(false);
            break;
        }
        return {};
    }
    static VkShaderStageFlagBits _GetShaderStage(GFXGpuProgramStageFlags flags)
    {
        std::underlying_type_t<VkShaderStageFlagBits> stage{};
        if (HasFlag(flags, GFXGpuProgramStageFlags::Vertex))
        {
            stage |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (HasFlag(flags, GFXGpuProgramStageFlags::Fragment))
        {
            stage |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        assert(stage != 0);
        return VkShaderStageFlagBits(stage);
    }

    GFXVulkanDescriptorSetLayout::GFXVulkanDescriptorSetLayout(
        GFXVulkanApplication* app,
        const GFXDescriptorLayoutDesc* layouts,
        size_t layoutCount)
        : m_app(app)
    {
        array_list<VkDescriptorSetLayoutBinding> bindings;
        for (size_t i = 0; i < layoutCount; ++i)
        {
            const auto layoutInfo = layouts[i];
            m_debugInfo.push_back(layoutInfo);

            VkDescriptorSetLayoutBinding& binding = bindings.emplace_back();
            binding.binding         = layoutInfo.BindingPoint;
            binding.descriptorType  = _GetDescriptorType(layoutInfo.Type);
            binding.descriptorCount = 1;
            binding.stageFlags      = _GetShaderStage(layoutInfo.Stage);
            binding.pImmutableSamplers = nullptr;

            // Tally per-type descriptor counts so this layout's own pool can be sized exactly.
            m_typeCounts[binding.descriptorType] += 1;

            // Sanity-check: log if a CombinedImageSampler layout ends up mapped to UNIFORM_BUFFER
            assert(!(layoutInfo.Type == GFXDescriptorType::CombinedImageSampler &&
                     binding.descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
        }
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutCreateInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(app->GetVkDevice(), &layoutCreateInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    GFXVulkanDescriptorSetLayout::~GFXVulkanDescriptorSetLayout()
    {
        // Destroying the pools implicitly frees every set ever allocated from them,
        // so recycled handles in m_freeSets need no individual vkFreeDescriptorSets.
        for (auto pool : m_pools)
        {
            vkDestroyDescriptorPool(m_app->GetVkDevice(), pool, nullptr);
        }
        m_pools.clear();

        if (m_descriptorSetLayout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(m_app->GetVkDevice(), m_descriptorSetLayout, nullptr);
        }
    }

    static constexpr uint32_t kSetsPerPool = 128;

    VkDescriptorPool GFXVulkanDescriptorSetLayout::CreatePool()
    {
        std::vector<VkDescriptorPoolSize> sizes;
        for (const auto& [type, count] : m_typeCounts)
        {
            sizes.push_back(VkDescriptorPoolSize{type, count * kSetsPerPool});
        }
        // An empty layout (zero bindings, used to keep set numbering aligned) still needs a
        // valid pool to allocate empty sets from; a token size keeps vkCreateDescriptorPool legal.
        if (sizes.empty())
        {
            sizes.push_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, kSetsPerPool});
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
        poolInfo.pPoolSizes = sizes.data();
        poolInfo.maxSets = kSetsPerPool;
        // No FREE_DESCRIPTOR_SET_BIT: sets are recycled via m_freeSets, never freed one-by-one.

        VkDescriptorPool pool = VK_NULL_HANDLE;
        if (vkCreateDescriptorPool(m_app->GetVkDevice(), &poolInfo, nullptr, &pool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
        return pool;
    }

    VkDescriptorSet GFXVulkanDescriptorSetLayout::AcquireVkSet()
    {
        std::lock_guard<std::mutex> lock(m_poolMutex);

        if (!m_freeSets.empty())
        {
            VkDescriptorSet recycled = m_freeSets.back();
            m_freeSets.pop_back();
            return recycled;
        }

        if (m_pools.empty() || m_currentPoolRemaining == 0)
        {
            m_pools.push_back(CreatePool());
            m_currentPoolRemaining = kSetsPerPool;
        }

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_pools.back();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_descriptorSetLayout;

        VkDescriptorSet set = VK_NULL_HANDLE;
        const auto result = vkAllocateDescriptorSets(m_app->GetVkDevice(), &allocInfo, &set);
        assert(result == VK_SUCCESS);
        --m_currentPoolRemaining;
        return set;
    }

    void GFXVulkanDescriptorSetLayout::RecycleVkSet(VkDescriptorSet set)
    {
        if (set == VK_NULL_HANDLE)
            return;
        std::lock_guard<std::mutex> lock(m_poolMutex);
        m_freeSets.push_back(set);
    }

    std::shared_ptr<GFXDescriptorSet> GFXVulkanDescriptorSetLayout::AllocateSet()
    {
        auto self = std::static_pointer_cast<GFXVulkanDescriptorSetLayout>(shared_from_this());
        VkDescriptorSet handle = AcquireVkSet();
        return std::shared_ptr<GFXVulkanDescriptorSet>(new GFXVulkanDescriptorSet(self, handle));
    }


    void GFXVulkanDescriptor::SetConstantBuffer(GFXBuffer* buffer)
    {
        const auto vkBuffer = static_cast<GFXVulkanBuffer*>(buffer);

        BufferInfo.buffer = vkBuffer->GetVkBuffer();
        BufferInfo.offset = 0;
        BufferInfo.range = vkBuffer->GetSize();

        WriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        WriteInfo.dstSet = m_descriptorSet->GetVkDescriptorSet();
        WriteInfo.dstBinding = m_bindingPoint;
        WriteInfo.dstArrayElement = 0;
        WriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        WriteInfo.descriptorCount = 1;
        WriteInfo.pBufferInfo = &BufferInfo;

        IsDirty = true;
    }
    void GFXVulkanDescriptor::SetConstantBufferDynamic(GFXBuffer* buffer)
    {
        const auto vkBuffer = static_cast<GFXVulkanBuffer*>(buffer);

        BufferInfo.buffer = vkBuffer->GetVkBuffer();
        BufferInfo.offset = 0;
        BufferInfo.range = vkBuffer->GetDesc().ElementSize; // range = single element, not whole buffer

        WriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        WriteInfo.dstSet = m_descriptorSet->GetVkDescriptorSet();
        WriteInfo.dstBinding = m_bindingPoint;
        WriteInfo.dstArrayElement = 0;
        WriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        WriteInfo.descriptorCount = 1;
        WriteInfo.pBufferInfo = &BufferInfo;

        IsDirty = true;
    }
    void GFXVulkanDescriptor::SetStructuredBuffer(GFXBuffer* buffer)
    {
        const auto vkBuffer = static_cast<GFXVulkanBuffer*>(buffer);

        BufferInfo.buffer = vkBuffer->GetVkBuffer();
        BufferInfo.offset = 0;
        BufferInfo.range = vkBuffer->GetSize();

        WriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        WriteInfo.dstSet = m_descriptorSet->GetVkDescriptorSet();
        WriteInfo.dstBinding = m_bindingPoint;
        WriteInfo.dstArrayElement = 0;
        WriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        WriteInfo.descriptorCount = 1;
        WriteInfo.pBufferInfo = &BufferInfo;

        IsDirty = true;
    }
    void GFXVulkanDescriptor::SetTextureSampler2D(GFXTexture2DView* texture)
    {
        auto vkView = dynamic_cast<GFXVulkanTexture2DView*>(texture);

        VkImageView imageView = vkView->GetVkImageView();
        VkSampler sampler = vkView->GetVkTexture()->GetVkSampler();

        ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageInfo.imageView = imageView;
        ImageInfo.sampler = sampler;

        WriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        WriteInfo.dstSet = m_descriptorSet->GetVkDescriptorSet();
        WriteInfo.dstBinding = m_bindingPoint;
        WriteInfo.dstArrayElement = 0;
        WriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        WriteInfo.descriptorCount = 1;
        WriteInfo.pImageInfo = &ImageInfo;

        IsDirty = true;
    }
    void GFXVulkanDescriptor::SetTexture2D(GFXTexture* texture)
    {
        VkImageView imageView{};
        if (texture->GetClassId() == typeid(GFXVulkanTexture))
        {
            auto tex2d = static_cast<GFXVulkanTexture*>(texture);
            imageView = tex2d->GetVkImageView();
        }
        else
        {
            assert(0);
        }

        ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ImageInfo.imageView = imageView;
        ImageInfo.sampler = nullptr;

        WriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        WriteInfo.dstSet = m_descriptorSet->GetVkDescriptorSet();
        WriteInfo.dstBinding = m_bindingPoint;
        WriteInfo.dstArrayElement = 0;
        WriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        WriteInfo.descriptorCount = 1;
        WriteInfo.pImageInfo = &ImageInfo;

        IsDirty = true;
    }


    GFXVulkanDescriptorSet::GFXVulkanDescriptorSet(const GFXDescriptorSetLayout_sp& layout, VkDescriptorSet handle)
        : m_descriptorSet(handle)
    {
        // The VkDescriptorSet is already allocated by the owning layout's pool chain.
        m_setlayout = std::static_pointer_cast<GFXVulkanDescriptorSetLayout>(layout);
    }

    GFXVulkanDescriptorSet::~GFXVulkanDescriptorSet()
    {
        m_descriptors.clear();

        // Return the handle to the owning layout's free list for reuse instead of freeing it.
        if (m_setlayout)
            m_setlayout->RecycleVkSet(m_descriptorSet);
        m_descriptorSet = VK_NULL_HANDLE;
    }

    GFXDescriptor* GFXVulkanDescriptorSet::AddDescriptor(std::string_view name, uint32_t bindingPoint)
    {
        auto descriptor = new GFXVulkanDescriptor{this, bindingPoint};
        descriptor->name = name;
        m_descriptors.push_back(std::unique_ptr<GFXVulkanDescriptor>{descriptor});
        return descriptor;
    }

    GFXDescriptor* GFXVulkanDescriptorSet::GetDescriptorAt(int index)
    {
        if (index >= m_descriptors.size())
            return nullptr;
        return m_descriptors[index].get();
    }
    int32_t GFXVulkanDescriptorSet::GetDescriptorCount() const
    {
        return m_descriptors.size();
    }
    GFXDescriptor* GFXVulkanDescriptorSet::Find(std::string_view name)
    {
        for (auto& item : m_descriptors)
        {
            if (item->name == name)
            {
                return item.get();
            }
        }
        return nullptr;
    }
    GFXDescriptor* GFXVulkanDescriptorSet::FindByBinding(uint32_t bindingPoint)
    {
        for (auto& item : m_descriptors)
        {
            if (item->GetBindingPoint() == bindingPoint)
            {
                return item.get();
            }
        }
        return nullptr;
    }
    void GFXVulkanDescriptorSet::Submit()
    {
        std::vector<VkWriteDescriptorSet> writeInfos;
        for (const auto& descriptor : m_descriptors)
        {
            if (descriptor->IsDirty)
            {
                writeInfos.push_back(descriptor->WriteInfo);
                descriptor->IsDirty = false;
            }
        }
        if (!writeInfos.empty())
        {
            vkUpdateDescriptorSets(
                GetApplication()->GetVkDevice(),
                static_cast<uint32_t>(writeInfos.size()),
                writeInfos.data(),
                0,
                nullptr);
        }
    }
    intptr_t GFXVulkanDescriptorSet::GetId()
    {
        return (intptr_t)m_descriptorSet;
    }
    GFXVulkanApplication* GFXVulkanDescriptorSet::GetApplication() const
    {
        return m_setlayout ? m_setlayout->GetApplication() : nullptr;
    }
    GFXDescriptorSetLayout_sp GFXVulkanDescriptorSet::GetDescriptorSetLayout() const
    {
        return m_setlayout;
    }

} // namespace gfx