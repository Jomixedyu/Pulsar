#pragma once
#include <gfx/GFXDescriptorSet.h>
#include "VulkanInclude.h"

namespace gfx
{
    class GFXVulkanApplication;
    class GFXVulkanDescriptorSet;
    class GFXVulkanDescriptorPool;

    class GFXVulkanDescriptorSetLayout : public GFXDescriptorSetLayout
    {
        using base = GFXDescriptorSetLayout;
    public:
        GFXVulkanDescriptorSetLayout(
            GFXVulkanApplication* app,
            const GFXDescriptorSetLayoutInfo* layouts,
            size_t layoutCount = 1);

        virtual ~GFXVulkanDescriptorSetLayout() override;

    public:
        const VkDescriptorSetLayout& GetVkDescriptorSetLayout() const { return m_descriptorSetLayout; }

    protected:
        array_list<GFXDescriptorSetLayoutInfo> m_debugInfo;
        VkDescriptorSetLayout m_descriptorSetLayout;
        GFXVulkanApplication* m_app;
    };
    GFX_DECL_SPTR(GFXVulkanDescriptorSetLayout);

    class GFXVulkanDescriptor : public GFXDescriptor
    {
    public:

        GFXVulkanDescriptor(GFXVulkanDescriptorSet* set, uint32_t bindingPoint)
            : m_descriptorSet(set), m_bindingPoint(bindingPoint)
        {

        }
        virtual void SetConstantBuffer(GFXBuffer* buffer) override;
        virtual void SetTextureSampler2D(GFXTexture* texture) override;
        virtual void SetTexture2D(GFXTexture* texture) override;

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
    GFX_DECL_SPTR(GFXVulkanDescriptor);

    class GFXVulkanDescriptorSet : public GFXDescriptorSet
    {
        using base = GFXDescriptorSet;
        friend class GFXVulkanDescriptorPool;
    private:
        GFXVulkanDescriptorSet(GFXVulkanDescriptorPool* pool, const GFXDescriptorSetLayout_sp& layout);
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
        GFXVulkanApplication* GetApplication() const;
        const VkDescriptorSet& GetVkDescriptorSet() const { return m_descriptorSet; }
        GFXVulkanDescriptorSetLayout_sp GetVkDescriptorSetLayout() const { return m_setlayout; }
        virtual GFXDescriptorSetLayout_sp GetDescriptorSetLayout() const override;
    protected:
        GFXVulkanDescriptorPool* m_pool;
        std::vector<std::unique_ptr<GFXVulkanDescriptor>> m_descriptors;
        VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
        GFXVulkanDescriptorSetLayout_sp m_setlayout;
    };
    GFX_DECL_SPTR(GFXVulkanDescriptorSet);

}