#include <cassert>
#include <gfx-vk/GFXVulkanApplication.h>
#include <gfx-vk/GFXVulkanResourceRegistry.h>
#include <gfx-vk/GFXVulkanBuffer.h>
#include <gfx-vk/GFXVulkanDescriptorSet.h>
#include <gfx-vk/GFXVulkanSampler.h>
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
        GFXResourceRegistry* registry,
        std::span<const GFXDescriptorLayoutDesc> layouts)
        : GFXDescriptorSetLayout(registry)
    {
        auto* app = GetApplication();
        array_list<VkDescriptorSetLayoutBinding> bindings;
        for (size_t i = 0; i < layouts.size(); ++i)
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

    GFXVulkanApplication* GFXVulkanDescriptorSetLayout::GetApplication() const
    {
        auto* registry = static_cast<GFXVulkanResourceRegistry*>(GetResourceRegistry());
        return registry ? registry->GetVulkanApplication() : nullptr;
    }

    GFXVulkanDescriptorSetLayout::~GFXVulkanDescriptorSetLayout()
    {
        // Destroying the pools implicitly frees every set ever allocated from them,
        // so recycled handles in the free lists need no individual vkFreeDescriptorSets.
        for (auto& pool : m_pools)
        {
            if (auto* app = GetApplication())
            {
                vkDestroyDescriptorPool(app->GetVkDevice(), pool->Handle, nullptr);
            }
        }
        m_pools.clear();
        m_setOwner.clear();

        if (m_descriptorSetLayout != VK_NULL_HANDLE)
        {
            if (auto* app = GetApplication())
            {
                vkDestroyDescriptorSetLayout(app->GetVkDevice(), m_descriptorSetLayout, nullptr);
            }
        }
    }

    std::unique_ptr<GFXVulkanDescriptorSetLayout::Pool> GFXVulkanDescriptorSetLayout::CreatePool(uint32_t setCount)
    {
        std::vector<VkDescriptorPoolSize> sizes;
        for (const auto& [type, count] : m_typeCounts)
        {
            sizes.push_back(VkDescriptorPoolSize{type, count * setCount});
        }
        // An empty layout (zero bindings, used to keep set numbering aligned) still needs a
        // valid pool to allocate empty sets from; a token size keeps vkCreateDescriptorPool legal.
        if (sizes.empty())
        {
            sizes.push_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, setCount});
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
        poolInfo.pPoolSizes = sizes.data();
        poolInfo.maxSets = setCount;
        // No FREE_DESCRIPTOR_SET_BIT: sets are recycled via per-pool free lists,
        // never freed one-by-one.

        auto* app = GetApplication();
        assert(app);

        auto pool = std::make_unique<Pool>();
        pool->MaxSets = setCount;
        if (vkCreateDescriptorPool(app->GetVkDevice(), &poolInfo, nullptr, &pool->Handle) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }

        // Allocate every set upfront into the free list (tag 0: never referenced, always
        // reusable): the recording path then never calls into the driver for allocation.
        std::vector<VkDescriptorSetLayout> layouts(setCount, m_descriptorSetLayout);
        std::vector<VkDescriptorSet> sets(setCount, VK_NULL_HANDLE);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pool->Handle;
        allocInfo.descriptorSetCount = setCount;
        allocInfo.pSetLayouts = layouts.data();

        if (vkAllocateDescriptorSets(app->GetVkDevice(), &allocInfo, sets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        // Fresh sets live in the free list only; m_setOwner tracks sets currently
        // handed out (acquired), which is what RecycleVkSet needs to route them home.
        for (VkDescriptorSet set : sets)
        {
            pool->FreeSets.emplace_back(set, 0);
        }
        return pool;
    }

    uint64_t GFXVulkanDescriptorSetLayout::GetCompletedTimeline() const
    {
        auto* app = GetApplication();
        uint64_t value = 0;
        vkGetSemaphoreCounterValue(app->GetVkDevice(), app->GetTimelineSemaphore(), &value);
        return value;
    }

    VkDescriptorSet GFXVulkanDescriptorSetLayout::AcquireVkSet()
    {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        const uint64_t completed = GetCompletedTimeline();

        GCIdlePools(completed);

        // Reuse the oldest free set whose referencing submissions have completed
        // (tag 0 = freshly allocated, always passes).
        for (auto& pool : m_pools)
        {
            if (pool->FreeSets.empty())
                continue;
            auto& [set, waitValue] = pool->FreeSets.front();
            if (completed >= waitValue)
            {
                VkDescriptorSet recycled = set;
                pool->FreeSets.pop_front();
                m_setOwner.emplace(recycled, pool.get());
                return recycled;
            }
        }

        // Everything is live: grow a new pool (4, 8, 16, ... sets).
        m_pools.push_back(CreatePool(m_nextPoolSets));
        m_nextPoolSets = std::min(m_nextPoolSets * 2, kMaxPoolSets);

        auto& pool = m_pools.back();
        VkDescriptorSet set = pool->FreeSets.front().first;
        pool->FreeSets.pop_front();
        m_setOwner.emplace(set, pool.get());
        return set;
    }

    void GFXVulkanDescriptorSetLayout::RecycleVkSet(VkDescriptorSet set)
    {
        if (set == VK_NULL_HANDLE)
            return;
        // Tag with the current frame and return to the owning pool; reuse is gated on
        // the in-flight window in AcquireVkSet.
        std::lock_guard<std::mutex> lock(m_poolMutex);
        auto it = m_setOwner.find(set);
        assert(it != m_setOwner.end());
        if (it == m_setOwner.end())
            return;
        Pool* pool = it->second;
        m_setOwner.erase(it);
        // Tag with the last submitted frame: the set is reusable once the timeline
        // reaches it (that submission's completion covers every possible reference).
        pool->FreeSets.emplace_back(set, GetApplication()->GetFrameCount());
    }

    void GFXVulkanDescriptorSetLayout::GCIdlePools(uint64_t completedTimeline)
    {
        auto* app = GetApplication();
        for (auto it = m_pools.begin(); it != m_pools.end();)
        {
            Pool& pool = **it;
            // Fully free and the newest retirement has completed on the GPU timeline:
            // no pending command can reference any set of this pool anymore.
            if (pool.MaxSets > 0 && pool.FreeSets.size() == pool.MaxSets &&
                completedTimeline >= pool.FreeSets.back().second)
            {
                for (const auto& [set, _] : pool.FreeSets)
                    m_setOwner.erase(set);
                vkDestroyDescriptorPool(app->GetVkDevice(), pool.Handle, nullptr);
                it = m_pools.erase(it);
                continue;
            }
            ++it;
        }
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

        // Samplers are no longer baked into textures; pull one from the shared
        // cache keyed on the texture's sampler config.
        auto app = m_descriptorSet->GetApplication();
        const auto& samplerCfg = vkView->GetVkTexture()->GetSamplerConfig();
        auto vkSampler = static_cast<GFXVulkanSampler*>(app->GetResourceRegistry()->GetBuiltinResources().GetSampler(samplerCfg));
        VkSampler sampler = vkSampler->GetVkSampler();

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


    GFXVulkanDescriptorSet::GFXVulkanDescriptorSet(const GFXDescriptorSetLayoutPtr& layout, VkDescriptorSet handle)
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
    GFXDescriptorSetLayoutPtr GFXVulkanDescriptorSet::GetDescriptorSetLayout() const
    {
        return m_setlayout;
    }

} // namespace gfx
