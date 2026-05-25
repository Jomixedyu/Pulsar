#pragma once
#include "GFXRefCountPtr.h"

namespace gfx
{
    // Legacy handle aliases — now defined as intrusive ref-count pointers.
    // All existing code using BufferHandle, TextureHandle, etc. should work
    // with minimal changes (just remove .index/.generation/.mgr access).
    //
    // Long-term goal: replace all uses of these aliases with direct
    // GFXRefCountPtr<T> declarations and then delete this file.

    class GFXBuffer;
    class GFXTexture;
    class GFXTextureView;
    class GFXFrameBufferObject;
    class GFXGraphicsPipeline;
    class GFXDescriptorSetLayout;
    class GFXDescriptorSet;
    class GFXGpuProgram;
    class GFXVertexLayoutDescription;
    class GFXCommandBuffer;

    using BufferHandle                = GFXRefCountPtr<GFXBuffer>;
    using TextureHandle               = GFXRefCountPtr<GFXTexture>;
    using TextureViewHandle           = GFXRefCountPtr<GFXTextureView>;
    using FrameBufferObjectHandle     = GFXRefCountPtr<GFXFrameBufferObject>;
    using GraphicsPipelineHandle      = GFXRefCountPtr<GFXGraphicsPipeline>;
    using DescriptorSetLayoutHandle   = GFXRefCountPtr<GFXDescriptorSetLayout>;
    using DescriptorSetHandle         = GFXRefCountPtr<GFXDescriptorSet>;
    using GpuProgramHandle            = GFXRefCountPtr<GFXGpuProgram>;
    using VertexLayoutDescriptionHandle = GFXRefCountPtr<GFXVertexLayoutDescription>;
    using CommandBufferHandle         = GFXRefCountPtr<GFXCommandBuffer>;
}
