#pragma once
#include "GFXBuffer.h"
#include "GFXTexture2D.h"
#include "GFXInclude.h"
#include "GFXRenderTarget.h"
#include <string_view>

namespace gfx
{
    enum class GFXDescriptorType
    {
        ConstantBuffer,
        CombinedImageSampler,
        Texture2D
    };
    enum class GFXShaderStageFlags : uint32_t
    {
        Vertex = 1,
        Fragment = 1 << 1,
        VertexFragment = Vertex | Fragment,
    };
    inline const char* to_string(GFXShaderStageFlags stage)
    {
        switch (stage)
        {
        case GFXShaderStageFlags::Vertex: return "Vertex";
        case GFXShaderStageFlags::Fragment: return "Fragment";
        case GFXShaderStageFlags::VertexFragment: return "VertexFragment";
        }
        return nullptr;
    }

    struct GFXDescriptorSetLayoutInfo final
    {
    public:
        uint32_t BindingPoint;
        uint32_t SpacePoint;
        GFXDescriptorType Type;
        GFXShaderStageFlags Stage;

        GFXDescriptorSetLayoutInfo(
            GFXDescriptorType type,
            GFXShaderStageFlags stage,
            uint32_t bindingPoint = 0,
            uint32_t spacePoint = 0)
            : Type(type), Stage(stage), BindingPoint(bindingPoint), SpacePoint(spacePoint)
        {
        }
    };

    class GFXDescriptorSetLayout
    {
    protected:
        GFXDescriptorSetLayout()
        {
        }
        virtual ~GFXDescriptorSetLayout() {}
    };
    GFX_DECL_SPTR(GFXDescriptorSetLayout);

    class GFXDescriptor
    {
    public:
        GFXDescriptor() {}
        virtual ~GFXDescriptor() {}
        GFXDescriptor(const GFXDescriptor&) = delete;

        virtual void SetConstantBuffer(GFXBuffer* buffer) = 0;
        virtual void SetTextureSampler2D(GFXTexture* texture) = 0;
        virtual void SetTexture2D(GFXTexture* texture) = 0;

        bool IsDirty;
        std::string name;
    };
    GFX_DECL_SPTR(GFXDescriptor);

    class GFXDescriptorSet
    {
    protected:
        GFXDescriptorSet() {}
        virtual ~GFXDescriptorSet() {}
    public:
        virtual GFXDescriptor* AddDescriptor(std::string_view name, uint32_t bindingPoint) = 0;
        virtual GFXDescriptor* GetDescriptorAt(int index) = 0;
        virtual int32_t GetDescriptorCount() const = 0;
        virtual GFXDescriptor* Find(std::string_view name) = 0;
        virtual GFXDescriptor* FindByBinding(uint32_t bindingPoint) = 0;
        virtual void Submit() = 0;
        virtual intptr_t GetId() = 0;
        virtual GFXDescriptorSetLayout_sp GetDescriptorSetLayout() const = 0;
    };
    GFX_DECL_SPTR(GFXDescriptorSet);

}