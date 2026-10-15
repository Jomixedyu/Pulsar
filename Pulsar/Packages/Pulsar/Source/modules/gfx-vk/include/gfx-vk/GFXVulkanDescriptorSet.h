#pragma once
#include <gfx/GFXDescriptorSet.h>
#include "VulkanInclude.h"

#include <deque>
#include <memory>
#include <span>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace gfx
{
    class GFXVulkanApplication;
    class GFXVulkanDescriptorSet;

    class GFXVulkanDescriptorSetLayout : public GFXDescriptorSetLayout
    {
        using base = GFXDescriptorSetLayout;
        friend class GFXVulkanDescriptorSet;
        friend class GFXVulkanDescriptor;
    public:
        GFXVulkanDescriptorSetLayout(
            GFXResourceRegistry* registry,
            std::span<const GFXDescriptorLayoutDesc> layouts);

        virtual ~GFXVulkanDescriptorSetLayout() override;

    public:
        const VkDescriptorSetLayout& GetVkDescriptorSetLayout() const { return m_descriptorSetLayout; }

        // Allocate a descriptor set from this layout's own, exactly-sized pool chain.
        // Uses shared_from_this() so the returned set holds the owning layout: the layout
        // (and its pools) outlive the set, and the set recycles itself on destroy.
        std::shared_ptr<GFXDescriptorSet> AllocateSet() override;

    private:
        struct Pool
        {
            VkDescriptorPool Handle = VK_NULL_HANDLE;
            uint32_t MaxSets = 0;
            // Free sets, oldest first. Tag = the graphics-queue timeline value that must
            // complete before reuse; freshly allocated sets are tagged 0 (never
            // referenced, always safe).
            std::deque<std::pair<VkDescriptorSet, uint64_t>> FreeSets;
        };

        VkDescriptorSet AcquireVkSet();          // unused set first, then an aged recycled set, else a new pool
        void RecycleVkSet(VkDescriptorSet set);  // return to the owning pool's free list (frame-tagged)
        // Creates a pool and allocates every set upfront into the unused list (no driver
        // allocation calls later on the recording path).
        std::unique_ptr<Pool> CreatePool(uint32_t setCount);
        // Destroys pools whose sets are all free and past the in-flight window.
        // Called from AcquireVkSet (render thread, under m_poolMutex).
        void GCIdlePools(uint64_t completedTimeline);
        // Graphics-queue timeline: completed submission count.
        uint64_t GetCompletedTimeline() const;
        GFXVulkanApplication* GetApplication() const;

    protected:
        array_list<GFXDescriptorLayoutDesc> m_debugInfo;
        VkDescriptorSetLayout m_descriptorSetLayout;
        // Per-layout descriptor pool: poolSize is computed exactly from this layout's bindings
        // (no global type-ratio guessing), grows by appending pools, recycles freed sets.
        std::unordered_map<VkDescriptorType, uint32_t> m_typeCounts; // descriptors per set, per type
        std::vector<std::unique_ptr<Pool>> m_pools;
        std::unordered_map<VkDescriptorSet, Pool*> m_setOwner;
        // Pools grow geometrically (4, 8, 16, ...) so small layouts do not pay for
        // a 128-set pool; GCIdlePools returns fully-idle pools to the driver.
        static constexpr uint32_t kFirstPoolSets = 4;
        static constexpr uint32_t kMaxPoolSets = 128;
        uint32_t m_nextPoolSets = kFirstPoolSets;
        std::mutex m_poolMutex;
    };
    GFX_DECL_PTR(GFXVulkanDescriptorSetLayout);

    class GFXVulkanDescriptor : public GFXDescriptor
    {
    public:

        GFXVulkanDescriptor(GFXVulkanDescriptorSet* set, uint32_t bindingPoint)
            : m_descriptorSet(set), m_bindingPoint(bindingPoint)
        {

        }
        virtual void SetConstantBuffer(GFXBuffer* buffer) override;
        virtual void SetConstantBufferDynamic(GFXBuffer* buffer) override;
        virtual void SetTextureSampler2D(GFXTexture2DView* texture) override;
        virtual void SetTexture2D(GFXTexture* texture) override;
        virtual void SetStructuredBuffer(GFXBuffer* buffer) override;

        uint32_t GetBindingPoint() const
        {
            return m_bindingPoint;
        }
        VkDescriptorBufferInfo BufferInfo{};
        VkDescriptorImageInfo ImageInfo{};
        VkWriteDescriptorSet WriteInfo{};
    protected:
        GFXVulkanDescriptorSet* m_descriptorSet;
        uint32_t m_bindingPoint;
    };
    GFX_DECL_PTR(GFXVulkanDescriptor);

    class GFXVulkanDescriptorSet : public GFXDescriptorSet
    {
        using base = GFXDescriptorSet;
        friend class GFXVulkanDescriptorSetLayout;
        friend class GFXVulkanDescriptor;
    private:
        GFXVulkanDescriptorSet(const GFXDescriptorSetLayoutPtr& layout, VkDescriptorSet handle);
    public:
        virtual ~GFXVulkanDescriptorSet() override;
        GFXVulkanDescriptorSet(const GFXVulkanDescriptorSet&) = delete;
    public:
        virtual GFXDescriptor* AddDescriptor(std::string_view name, uint32_t bindingPoint) override;
        virtual GFXDescriptor* GetDescriptorAt(int index) override;
        virtual int32_t GetDescriptorCount() const override;
        virtual GFXDescriptor* Find(std::string_view name) override;
        virtual GFXDescriptor* FindByBinding(uint32_t bindingPoint) override;
        virtual void Submit() override;
        virtual intptr_t GetId() override;
    public:
        const VkDescriptorSet& GetVkDescriptorSet() const { return m_descriptorSet; }
        GFXVulkanDescriptorSetLayoutPtr GetVkDescriptorSetLayout() const { return m_setlayout; }
        virtual GFXDescriptorSetLayoutPtr GetDescriptorSetLayout() const override;

    private:
        GFXVulkanApplication* GetApplication() const;

    protected:
        std::vector<std::unique_ptr<GFXVulkanDescriptor>> m_descriptors;
        VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
        GFXVulkanDescriptorSetLayoutPtr m_setlayout;
    };
    GFX_DECL_PTR(GFXVulkanDescriptorSet);

}
