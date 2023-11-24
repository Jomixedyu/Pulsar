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
            const std::vector<GFXDescriptorSetLayoutInfo>& layout);

        virtual ~GFXVulkanDescriptorSetLayout() override;

    public:
        const VkDescriptorSetLayout& GetVkDescriptorSetLayout() const { return m_descriptorSetLayout; }

    protected:
        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        GFXVulkanApplication* m_app;
    };

    class GFXVulkanDescriptor : public GFXDescriptor
    {
    public:

        GFXVulkanDescriptor(GFXVulkanDescriptorSet* set, uint32_t bindingPoint)
            : m_descriptorSet(set), m_bindingPoint(bindingPoint)
        {

        }
        virtual void SetConstantBuffer(GFXBuffer* buffer) override;
        virtual void SetTextureSampler2D(GFXTexture* texture) override;
        uint32_t GetBindingPoint() const { return m_bindingPoint; }
    public:
        VkDescriptorBufferInfo BufferInfo{};
        VkDescriptorImageInfo ImageInfo{};
        VkWriteDescriptorSet WriteInfo{};
    protected:
        GFXVulkanDescriptorSet* m_descriptorSet;
        uint32_t m_bindingPoint;
    };


    class GFXVulkanDescriptorSet : public GFXDescriptorSet
    {
        using base = GFXDescriptorSet;
        friend class GFXVulkanDescriptorPool;
    private:
        GFXVulkanDescriptorSet(GFXVulkanDescriptorPool* pool, GFXDescriptorSetLayout* layout);
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
    protected:
        GFXVulkanDescriptorPool* m_pool;
        std::vector<std::unique_ptr<GFXVulkanDescriptor>> m_descriptors;
        VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
        GFXVulkanDescriptorSetLayout* m_setlayout;
    };

}