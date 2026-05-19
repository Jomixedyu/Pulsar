#pragma once
#include <cstdint>
#include <type_traits>

namespace gfx
{
    static constexpr uint32_t kInvalidHandleIndex = UINT32_MAX;

    // Strongly-typed handle with generation counter for use-after-free detection.
    // T is a tag type (e.g., struct BufferHandleTag) to prevent mixing handles.
    template<typename T>
    struct GFXHandle
    {
        uint32_t index = kInvalidHandleIndex;
        uint16_t generation = 0;

        bool IsValid() const { return index != kInvalidHandleIndex; }
        void Invalidate() { index = kInvalidHandleIndex; generation = 0; }

        bool operator==(const GFXHandle& other) const
        {
            return index == other.index && generation == other.generation;
        }
        bool operator!=(const GFXHandle& other) const { return !(*this == other); }
    };

    // Alias for specific resource handle types.
    struct BufferHandleTag {};
    struct TextureHandleTag {};
    struct TextureViewHandleTag {};
    struct FrameBufferObjectHandleTag {};
    struct GraphicsPipelineHandleTag {};
    struct DescriptorSetLayoutHandleTag {};
    struct DescriptorSetHandleTag {};
    struct GpuProgramHandleTag {};
    struct VertexLayoutDescriptionHandleTag {};
    struct CommandBufferHandleTag {};

    using BufferHandle                = GFXHandle<BufferHandleTag>;
    using TextureHandle               = GFXHandle<TextureHandleTag>;
    using TextureViewHandle           = GFXHandle<TextureViewHandleTag>;
    using FrameBufferObjectHandle     = GFXHandle<FrameBufferObjectHandleTag>;
    using GraphicsPipelineHandle      = GFXHandle<GraphicsPipelineHandleTag>;
    using DescriptorSetLayoutHandle   = GFXHandle<DescriptorSetLayoutHandleTag>;
    using DescriptorSetHandle         = GFXHandle<DescriptorSetHandleTag>;
    using GpuProgramHandle            = GFXHandle<GpuProgramHandleTag>;
    using VertexLayoutDescriptionHandle = GFXHandle<VertexLayoutDescriptionHandleTag>;
    using CommandBufferHandle         = GFXHandle<CommandBufferHandleTag>;

} // namespace gfx
