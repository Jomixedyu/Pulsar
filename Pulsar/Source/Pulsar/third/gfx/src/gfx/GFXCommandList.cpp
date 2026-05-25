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

GFXRefCountPtr<GFXBuffer> GFXCommandListImmediate::CreateBuffer(const GFXBufferDesc& desc)
{
    return m_resourceManager->CreateBuffer(desc);
}

GFXRefCountPtr<GFXTexture> GFXCommandListImmediate::CreateTexture2D(const GFXTextureCreateDesc& desc)
{
    return m_resourceManager->CreateTexture2D(desc);
}

GFXRefCountPtr<GFXTexture> GFXCommandListImmediate::CreateTextureCube(int32_t size)
{
    return m_resourceManager->CreateTextureCube(size);
}

GFXRefCountPtr<GFXTexture> GFXCommandListImmediate::CreateRenderTarget(const GFXTextureCreateDesc& desc)
{
    return m_resourceManager->CreateRenderTarget(desc);
}

GFXRefCountPtr<GFXFrameBufferObject> GFXCommandListImmediate::CreateFrameBufferObject(const array_list<GFXTexture2DView_sp>& attachments)
{
    return m_resourceManager->CreateFrameBufferObject(attachments);
}

GFXRefCountPtr<GFXGpuProgram> GFXCommandListImmediate::CreateGpuProgram(GFXGpuProgramStageFlags stage, const void* code, size_t length)
{
    return m_resourceManager->CreateGpuProgram(stage, code, length);
}

GFXRefCountPtr<GFXDescriptorSetLayout> GFXCommandListImmediate::CreateDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutDesc>& bindings)
{
    return m_resourceManager->CreateDescriptorSetLayout(bindings);
}

GFXRefCountPtr<GFXVertexLayoutDescription> GFXCommandListImmediate::CreateVertexLayoutDescription()
{
    return m_resourceManager->CreateVertexLayoutDescription();
}

void GFXCommandListImmediate::UploadBuffer(GFXBuffer* buffer, const void* data, size_t size)
{
    if (m_resourceManager && buffer && data && size > 0)
    {
        m_resourceManager->UploadBuffer(buffer, data, size);
    }
}

void GFXCommandListImmediate::UploadTexture(GFXTexture* texture, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format)
{
    if (m_resourceManager && texture && data)
    {
        m_resourceManager->UploadTexture(texture, data, width, height, format);
    }
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

GFXRefCountPtr<GFXBuffer> GFXCommandListDeferred::CreateBuffer(const GFXBufferDesc& desc)
{
    return m_resourceManager->CreateBuffer(desc);
}

GFXRefCountPtr<GFXTexture> GFXCommandListDeferred::CreateTexture2D(const GFXTextureCreateDesc& desc)
{
    return m_resourceManager->CreateTexture2D(desc);
}

GFXRefCountPtr<GFXTexture> GFXCommandListDeferred::CreateTextureCube(int32_t size)
{
    return m_resourceManager->CreateTextureCube(size);
}

GFXRefCountPtr<GFXTexture> GFXCommandListDeferred::CreateRenderTarget(const GFXTextureCreateDesc& desc)
{
    return m_resourceManager->CreateRenderTarget(desc);
}

GFXRefCountPtr<GFXFrameBufferObject> GFXCommandListDeferred::CreateFrameBufferObject(const array_list<GFXTexture2DView_sp>& attachments)
{
    return m_resourceManager->CreateFrameBufferObject(attachments);
}

GFXRefCountPtr<GFXGpuProgram> GFXCommandListDeferred::CreateGpuProgram(GFXGpuProgramStageFlags stage, const void* code, size_t length)
{
    return m_resourceManager->CreateGpuProgram(stage, code, length);
}

GFXRefCountPtr<GFXDescriptorSetLayout> GFXCommandListDeferred::CreateDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutDesc>& bindings)
{
    return m_resourceManager->CreateDescriptorSetLayout(bindings);
}

GFXRefCountPtr<GFXVertexLayoutDescription> GFXCommandListDeferred::CreateVertexLayoutDescription()
{
    return m_resourceManager->CreateVertexLayoutDescription();
}

void GFXCommandListDeferred::UploadBuffer(GFXBuffer* buffer, const void* data, size_t size)
{
    if (m_resourceManager && buffer && data && size > 0)
    {
        m_resourceManager->UploadBuffer(buffer, data, size);
    }
}

void GFXCommandListDeferred::UploadTexture(GFXTexture* texture, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format)
{
    if (m_resourceManager && texture && data)
    {
        m_resourceManager->UploadTexture(texture, data, width, height, format);
    }
}

} // namespace gfx
