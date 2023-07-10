#pragma once
#include <cstdint>
#include <vector>
#include "GFXBuffer.h"
#include "GFXTexture2D.h"

namespace gfx
{
    enum class GFXDescriptorType
    {
        ConstantBuffer,
        CombinedImageSampler
    };
    enum class GFXShaderStage : uint32_t
    {
        Vertex = 1,
        Fragment = 16,
    };

    struct GFXDescriptorSetLayoutInfo
    {
    public:
        uint32_t BindingPoint;
        GFXDescriptorType Type;
        GFXShaderStage Stage;
    };

    class GFXDescriptorSetLayout
    {
    protected:
        GFXDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutInfo>& layout)
            : m_layout(layout)
        {
        }
        virtual ~GFXDescriptorSetLayout() {}

    protected:
        std::vector<GFXDescriptorSetLayoutInfo> m_layout;
    };

    class GFXDescriptor
    {
    public:
        GFXDescriptor() {}
        virtual ~GFXDescriptor() {}
        GFXDescriptor(const GFXDescriptor&) = delete;

        virtual void SetConstantBuffer(size_t size, GFXBuffer* buffer) = 0;
        virtual void SetTextureSampler2D(GFXTexture2D* texture) = 0;

    };

    class GFXDescriptorSet
    {
    protected:
        GFXDescriptorSet() {}
        virtual ~GFXDescriptorSet() {}
    public:
        virtual GFXDescriptor* AddDescriptor(uint32_t bindingPoint) = 0;
        virtual void Submit() = 0;
    };
}