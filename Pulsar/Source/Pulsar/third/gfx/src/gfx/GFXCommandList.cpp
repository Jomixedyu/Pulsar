#include "gfx/GFXCommandList.h"

namespace gfx
{

// ==========================================================================
// GFXCommandListImmediate
// ==========================================================================

GFXCommandListImmediate::GFXCommandListImmediate(GFXResourceManager* resMgr)
{
    m_resourceManager = resMgr;
}

void GFXCommandListImmediate::Enqueue(Command cmd)
{
    cmd(*this);
}

void GFXCommandListImmediate::Flush()
{
    // no-op — everything is executed immediately
}

void GFXCommandListImmediate::BeginFrame(uint64_t frameIndex)
{
    m_resourceManager->BeginFrame(frameIndex);
}

void GFXCommandListImmediate::EndFrame(uint64_t frameIndex)
{
    m_resourceManager->EndFrame(frameIndex);
}

BufferHandle GFXCommandListImmediate::CreateBuffer(const GFXBufferDesc& desc)
{
    auto handle = m_resourceManager->AllocHandle<BufferHandle>();
    m_resourceManager->CreateBuffer(handle, desc);
    return handle;
}

TextureHandle GFXCommandListImmediate::CreateTexture2D(const GFXTextureCreateDesc& desc)
{
    auto handle = m_resourceManager->AllocHandle<TextureHandle>();
    m_resourceManager->CreateTexture2D(handle, desc);
    return handle;
}

TextureHandle GFXCommandListImmediate::CreateTextureCube(int32_t size)
{
    auto handle = m_resourceManager->AllocHandle<TextureHandle>();
    m_resourceManager->CreateTextureCube(handle, size);
    return handle;
}

TextureHandle GFXCommandListImmediate::CreateRenderTarget(const GFXTextureCreateDesc& desc)
{
    auto handle = m_resourceManager->AllocHandle<TextureHandle>();
    m_resourceManager->CreateRenderTarget(handle, desc);
    return handle;
}

FrameBufferObjectHandle GFXCommandListImmediate::CreateFrameBufferObject(const array_list<GFXTexture2DView_sp>& attachments)
{
    auto handle = m_resourceManager->AllocHandle<FrameBufferObjectHandle>();
    m_resourceManager->CreateFrameBufferObject(handle, attachments);
    return handle;
}

GpuProgramHandle GFXCommandListImmediate::CreateGpuProgram(GFXGpuProgramStageFlags stage, const void* code, size_t length)
{
    auto handle = m_resourceManager->AllocHandle<GpuProgramHandle>();
    m_resourceManager->CreateGpuProgram(handle, stage, code, length);
    return handle;
}

DescriptorSetLayoutHandle GFXCommandListImmediate::CreateDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutDesc>& bindings)
{
    auto handle = m_resourceManager->AllocHandle<DescriptorSetLayoutHandle>();
    m_resourceManager->CreateDescriptorSetLayout(handle, bindings);
    return handle;
}

VertexLayoutDescriptionHandle GFXCommandListImmediate::CreateVertexLayoutDescription()
{
    auto handle = m_resourceManager->AllocHandle<VertexLayoutDescriptionHandle>();
    m_resourceManager->CreateVertexLayoutDescription(handle);
    return handle;
}

void GFXCommandListImmediate::Destroy(BufferHandle handle)
{
    m_resourceManager->DestroyResource(handle.index, handle.generation);
}

void GFXCommandListImmediate::Destroy(TextureHandle handle)
{
    m_resourceManager->DestroyResource(handle.index, handle.generation);
}

void GFXCommandListImmediate::Destroy(FrameBufferObjectHandle handle)
{
    m_resourceManager->DestroyResource(handle.index, handle.generation);
}

void GFXCommandListImmediate::Destroy(GpuProgramHandle handle)
{
    m_resourceManager->DestroyResource(handle.index, handle.generation);
}

void GFXCommandListImmediate::Destroy(DescriptorSetLayoutHandle handle)
{
    m_resourceManager->DestroyResource(handle.index, handle.generation);
}

void GFXCommandListImmediate::Destroy(VertexLayoutDescriptionHandle handle)
{
    m_resourceManager->DestroyResource(handle.index, handle.generation);
}

void GFXCommandListImmediate::UploadBuffer(BufferHandle handle, const void* data, size_t size)
{
    if (m_resourceManager && data && size > 0)
    {
        m_resourceManager->UploadBuffer(handle, data, size);
    }
}

void GFXCommandListImmediate::UploadTexture(TextureHandle handle, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format)
{
    // TODO: implement when texture upload path exists in resource manager
}

// ==========================================================================
// GFXCommandListDeferred
// ==========================================================================

GFXCommandListDeferred::GFXCommandListDeferred(GFXResourceManager* resMgr)
{
    m_resourceManager = resMgr;
}

void GFXCommandListDeferred::Enqueue(Command cmd)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push_back(std::move(cmd));
}

void GFXCommandListDeferred::Flush()
{
    std::vector<Command> queue;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        queue.swap(m_queue);
    }
    for (auto& cmd : queue)
    {
        cmd(*this);
    }
}

void GFXCommandListDeferred::BeginFrame(uint64_t frameIndex)
{
    m_resourceManager->BeginFrame(frameIndex);
}

void GFXCommandListDeferred::EndFrame(uint64_t frameIndex)
{
    m_resourceManager->EndFrame(frameIndex);
}

BufferHandle GFXCommandListDeferred::CreateBuffer(const GFXBufferDesc& desc)
{
    auto handle = m_resourceManager->AllocHandle<BufferHandle>();
    m_resourceManager->CreateBuffer(handle, desc);
    return handle;
}

TextureHandle GFXCommandListDeferred::CreateTexture2D(const GFXTextureCreateDesc& desc)
{
    auto handle = m_resourceManager->AllocHandle<TextureHandle>();
    m_resourceManager->CreateTexture2D(handle, desc);
    return handle;
}

TextureHandle GFXCommandListDeferred::CreateTextureCube(int32_t size)
{
    auto handle = m_resourceManager->AllocHandle<TextureHandle>();
    m_resourceManager->CreateTextureCube(handle, size);
    return handle;
}

TextureHandle GFXCommandListDeferred::CreateRenderTarget(const GFXTextureCreateDesc& desc)
{
    auto handle = m_resourceManager->AllocHandle<TextureHandle>();
    m_resourceManager->CreateRenderTarget(handle, desc);
    return handle;
}

FrameBufferObjectHandle GFXCommandListDeferred::CreateFrameBufferObject(const array_list<GFXTexture2DView_sp>& attachments)
{
    auto handle = m_resourceManager->AllocHandle<FrameBufferObjectHandle>();
    m_resourceManager->CreateFrameBufferObject(handle, attachments);
    return handle;
}

GpuProgramHandle GFXCommandListDeferred::CreateGpuProgram(GFXGpuProgramStageFlags stage, const void* code, size_t length)
{
    auto handle = m_resourceManager->AllocHandle<GpuProgramHandle>();
    m_resourceManager->CreateGpuProgram(handle, stage, code, length);
    return handle;
}

DescriptorSetLayoutHandle GFXCommandListDeferred::CreateDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutDesc>& bindings)
{
    auto handle = m_resourceManager->AllocHandle<DescriptorSetLayoutHandle>();
    m_resourceManager->CreateDescriptorSetLayout(handle, bindings);
    return handle;
}

VertexLayoutDescriptionHandle GFXCommandListDeferred::CreateVertexLayoutDescription()
{
    auto handle = m_resourceManager->AllocHandle<VertexLayoutDescriptionHandle>();
    m_resourceManager->CreateVertexLayoutDescription(handle);
    return handle;
}

void GFXCommandListDeferred::Destroy(BufferHandle handle)
{
    m_resourceManager->DestroyResource(handle.index, handle.generation);
}

void GFXCommandListDeferred::Destroy(TextureHandle handle)
{
    m_resourceManager->DestroyResource(handle.index, handle.generation);
}

void GFXCommandListDeferred::Destroy(FrameBufferObjectHandle handle)
{
    m_resourceManager->DestroyResource(handle.index, handle.generation);
}

void GFXCommandListDeferred::Destroy(GpuProgramHandle handle)
{
    m_resourceManager->DestroyResource(handle.index, handle.generation);
}

void GFXCommandListDeferred::Destroy(DescriptorSetLayoutHandle handle)
{
    m_resourceManager->DestroyResource(handle.index, handle.generation);
}

void GFXCommandListDeferred::Destroy(VertexLayoutDescriptionHandle handle)
{
    m_resourceManager->DestroyResource(handle.index, handle.generation);
}

void GFXCommandListDeferred::UploadBuffer(BufferHandle handle, const void* data, size_t size)
{
    if (m_resourceManager && data && size > 0)
    {
        m_resourceManager->UploadBuffer(handle, data, size);
    }
}

void GFXCommandListDeferred::UploadTexture(TextureHandle handle, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format)
{
    // TODO: implement when texture upload path exists in resource manager
}

} // namespace gfx
