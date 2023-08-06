#pragma once
#include <cstdint>
#include <vector>
#include "GFXBuffer.h"
#include "GFXTexture2D.h"
#include "GFXInclude.h"
#include "GFXRenderTarget.h"

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
        VertexFragment = Vertex | Fragment,
    };

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
    GFX_DECL_SPTR(GFXDescriptorSetLayout);

    class GFXDescriptor
    {
    public:
        GFXDescriptor() {}
        virtual ~GFXDescriptor() {}
        GFXDescriptor(const GFXDescriptor&) = delete;

        virtual void SetConstantBuffer(size_t size, GFXBuffer* buffer) = 0;
        virtual void SetTextureSampler2D(GFXTexture* texture) = 0;

    };
    GFX_DECL_SPTR(GFXDescriptor);

    class GFXDescriptorSet
    {
    protected:
        GFXDescriptorSet() {}
        virtual ~GFXDescriptorSet() {}
    public:
        virtual GFXDescriptor* AddDescriptor(uint32_t bindingPoint) = 0;
        virtual void Submit() = 0;
        virtual intptr_t GetId() = 0;
    };
    GFX_DECL_SPTR(GFXDescriptorSet);

}