#pragma once
#include <cstdint>

namespace gfx
{
    enum class GFXResourceType : uint8_t
    {
        None = 0,
        Buffer,
        Texture,
        TextureView,
        FrameBufferObject,
        GraphicsPipeline,
        ComputePipeline,
        DescriptorSetLayout,
        DescriptorSet,
        GpuProgram,
        VertexLayoutDescription,
        CommandBuffer,
        Swapchain,
        RenderPass,
        Sampler,
        Count
    };
} // namespace gfx
