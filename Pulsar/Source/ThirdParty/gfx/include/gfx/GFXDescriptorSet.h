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
    enum class GFXShaderStageFlags : uint32_t
    {
        Vertex = 1,
        Fragment = 1 << 1,
    };
    inline GFXShaderStageFlags operator &(GFXShaderStageFlags a, GFXShaderStageFlags b)
    {
        return GFXShaderStageFlags(std::underlying_type_t<GFXShaderStageFlags>(a) & std::underlying_type_t<GFXShaderStageFlags>(b));
    }
    inline GFXShaderStageFlags operator |(GFXShaderStageFlags a, GFXShaderStageFlags b)
    {
        return GFXShaderStageFlags(std::underlying_type_t<GFXShaderStageFlags>(a) | std::underlying_type_t<GFXShaderStageFlags>(b));
    }
    inline bool HasFlags(GFXShaderStageFlags a, GFXShaderStageFlags b)
    {
        return (bool)std::underlying_type_t<GFXShaderStageFlags>(a & b);
    }

    struct GFXDescriptorSetLayoutInfo
    {
    public:
        uint32_t BindingPoint;
        GFXDescriptorType Type;
        GFXShaderStageFlags Stage;

        GFXDescriptorSetLayoutInfo(uint32_t bindingPoint, GFXDescriptorType type, GFXShaderStageFlags stage)
            : BindingPoint(bindingPoint), Type(type), Stage(stage)
        {
        }
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