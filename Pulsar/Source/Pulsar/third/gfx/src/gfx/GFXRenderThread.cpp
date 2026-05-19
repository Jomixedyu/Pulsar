#include "gfx/GFXRenderThread.h"

namespace gfx
{
    GFXRenderThread::GFXRenderThread(GFXApplication* app)
        : m_resourceManager(std::make_unique<GFXResourceManager>(app))
    {
    }

    GFXRenderThread::~GFXRenderThread() = default;

    // -------------------------------------------------------------------------
    // Deferred submission
    // -------------------------------------------------------------------------
    void GFXRenderThread::Enqueue(Command cmd)
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_deferredQueue.push_back(std::move(cmd));
    }

    BufferHandle GFXRenderThread::EnqueueCreateBuffer(const GFXBufferDesc& desc)
    {
        auto handle = m_resourceManager->AllocHandle<BufferHandle>();
        Enqueue([handle, desc](GFXResourceManager& mgr) {
            mgr.CreateBuffer(handle, desc);
        });
        return handle;
    }

    TextureHandle GFXRenderThread::EnqueueCreateTexture2D(const GFXTextureCreateDesc& desc)
    {
        auto handle = m_resourceManager->AllocHandle<TextureHandle>();
        // Copy image data into lambda if present
        std::vector<uint8_t> imageDataCopy;
        if (desc.ImageData && desc.DataLength > 0)
        {
            imageDataCopy.assign(desc.ImageData, desc.ImageData + desc.DataLength);
        }
        Enqueue([handle, desc, imageDataCopy = std::move(imageDataCopy)](GFXResourceManager& mgr) mutable {
            GFXTextureCreateDesc localDesc = desc;
            if (!imageDataCopy.empty())
            {
                localDesc.ImageData = imageDataCopy.data();
            }
            mgr.CreateTexture2D(handle, localDesc);
        });
        return handle;
    }

    TextureHandle GFXRenderThread::EnqueueCreateTextureCube(int32_t size)
    {
        auto handle = m_resourceManager->AllocHandle<TextureHandle>();
        Enqueue([handle, size](GFXResourceManager& mgr) {
            mgr.CreateTextureCube(handle, size);
        });
        return handle;
    }

    TextureHandle GFXRenderThread::EnqueueCreateRenderTarget(const GFXTextureCreateDesc& desc)
    {
        auto handle = m_resourceManager->AllocHandle<TextureHandle>();
        Enqueue([handle, desc](GFXResourceManager& mgr) {
            mgr.CreateRenderTarget(handle, desc);
        });
        return handle;
    }

    FrameBufferObjectHandle GFXRenderThread::EnqueueCreateFrameBufferObject(const array_list<GFXTexture2DView_sp>& attachments)
    {
        auto handle = m_resourceManager->AllocHandle<FrameBufferObjectHandle>();
        Enqueue([handle, attachments](GFXResourceManager& mgr) {
            mgr.CreateFrameBufferObject(handle, attachments);
        });
        return handle;
    }

    GpuProgramHandle GFXRenderThread::EnqueueCreateGpuProgram(GFXGpuProgramStageFlags stage, const void* code, size_t length)
    {
        auto handle = m_resourceManager->AllocHandle<GpuProgramHandle>();
        std::vector<uint8_t> codeCopy(static_cast<const uint8_t*>(code), static_cast<const uint8_t*>(code) + length);
        Enqueue([handle, stage, codeCopy = std::move(codeCopy)](GFXResourceManager& mgr) {
            mgr.CreateGpuProgram(handle, stage, codeCopy.data(), codeCopy.size());
        });
        return handle;
    }

    DescriptorSetLayoutHandle GFXRenderThread::EnqueueCreateDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutDesc>& bindings)
    {
        auto handle = m_resourceManager->AllocHandle<DescriptorSetLayoutHandle>();
        Enqueue([handle, bindings](GFXResourceManager& mgr) {
            mgr.CreateDescriptorSetLayout(handle, bindings);
        });
        return handle;
    }

    VertexLayoutDescriptionHandle GFXRenderThread::EnqueueCreateVertexLayoutDescription()
    {
        auto handle = m_resourceManager->AllocHandle<VertexLayoutDescriptionHandle>();
        Enqueue([handle](GFXResourceManager& mgr) {
            mgr.CreateVertexLayoutDescription(handle);
        });
        return handle;
    }

    void GFXRenderThread::EnqueueDestroy(BufferHandle handle)
    {
        if (!handle.IsValid()) return;
        Enqueue([handle](GFXResourceManager& mgr) {
            mgr.DestroyResource(handle.index, handle.generation);
        });
    }

    void GFXRenderThread::EnqueueDestroy(TextureHandle handle)
    {
        if (!handle.IsValid()) return;
        Enqueue([handle](GFXResourceManager& mgr) {
            mgr.DestroyResource(handle.index, handle.generation);
        });
    }

    void GFXRenderThread::EnqueueDestroy(FrameBufferObjectHandle handle)
    {
        if (!handle.IsValid()) return;
        Enqueue([handle](GFXResourceManager& mgr) {
            mgr.DestroyResource(handle.index, handle.generation);
        });
    }

    void GFXRenderThread::EnqueueDestroy(GpuProgramHandle handle)
    {
        if (!handle.IsValid()) return;
        Enqueue([handle](GFXResourceManager& mgr) {
            mgr.DestroyResource(handle.index, handle.generation);
        });
    }

    void GFXRenderThread::EnqueueDestroy(DescriptorSetLayoutHandle handle)
    {
        if (!handle.IsValid()) return;
        Enqueue([handle](GFXResourceManager& mgr) {
            mgr.DestroyResource(handle.index, handle.generation);
        });
    }

    void GFXRenderThread::EnqueueDestroy(VertexLayoutDescriptionHandle handle)
    {
        if (!handle.IsValid()) return;
        Enqueue([handle](GFXResourceManager& mgr) {
            mgr.DestroyResource(handle.index, handle.generation);
        });
    }

    void GFXRenderThread::EnqueueUploadBuffer(BufferHandle handle, const void* data, size_t size)
    {
        if (!handle.IsValid()) return;
        std::vector<uint8_t> dataCopy(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + size);
        Enqueue([handle, dataCopy = std::move(dataCopy)](GFXResourceManager& mgr) {
            mgr.UploadBuffer(handle, dataCopy.data(), dataCopy.size());
        });
    }

    void GFXRenderThread::EnqueueUploadTexture(TextureHandle handle, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format)
    {
        if (!handle.IsValid()) return;
        size_t byteSize = width * height * 4;
        std::vector<uint8_t> dataCopy(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + byteSize);
        Enqueue([handle, dataCopy = std::move(dataCopy), width, height, format](GFXResourceManager& mgr) {
            mgr.UploadTexture(handle, dataCopy.data(), width, height, format);
        });
    }

    // -------------------------------------------------------------------------
    // Immediate execution
    // -------------------------------------------------------------------------
    void GFXRenderThread::Execute(Command cmd)
    {
        cmd(*m_resourceManager);
    }

    BufferHandle GFXRenderThread::CreateBufferImmediate(const GFXBufferDesc& desc)
    {
        auto handle = m_resourceManager->AllocHandle<BufferHandle>();
        m_resourceManager->CreateBuffer(handle, desc);
        return handle;
    }

    TextureHandle GFXRenderThread::CreateTexture2DImmediate(const GFXTextureCreateDesc& desc)
    {
        auto handle = m_resourceManager->AllocHandle<TextureHandle>();
        m_resourceManager->CreateTexture2D(handle, desc);
        return handle;
    }

    TextureHandle GFXRenderThread::CreateTextureCubeImmediate(int32_t size)
    {
        auto handle = m_resourceManager->AllocHandle<TextureHandle>();
        m_resourceManager->CreateTextureCube(handle, size);
        return handle;
    }

    TextureHandle GFXRenderThread::CreateRenderTargetImmediate(const GFXTextureCreateDesc& desc)
    {
        auto handle = m_resourceManager->AllocHandle<TextureHandle>();
        m_resourceManager->CreateRenderTarget(handle, desc);
        return handle;
    }

    FrameBufferObjectHandle GFXRenderThread::CreateFrameBufferObjectImmediate(const array_list<GFXTexture2DView_sp>& attachments)
    {
        auto handle = m_resourceManager->AllocHandle<FrameBufferObjectHandle>();
        m_resourceManager->CreateFrameBufferObject(handle, attachments);
        return handle;
    }

    GpuProgramHandle GFXRenderThread::CreateGpuProgramImmediate(GFXGpuProgramStageFlags stage, const void* code, size_t length)
    {
        auto handle = m_resourceManager->AllocHandle<GpuProgramHandle>();
        m_resourceManager->CreateGpuProgram(handle, stage, code, length);
        return handle;
    }

    DescriptorSetLayoutHandle GFXRenderThread::CreateDescriptorSetLayoutImmediate(const std::vector<GFXDescriptorSetLayoutDesc>& bindings)
    {
        auto handle = m_resourceManager->AllocHandle<DescriptorSetLayoutHandle>();
        m_resourceManager->CreateDescriptorSetLayout(handle, bindings);
        return handle;
    }

    VertexLayoutDescriptionHandle GFXRenderThread::CreateVertexLayoutDescriptionImmediate()
    {
        auto handle = m_resourceManager->AllocHandle<VertexLayoutDescriptionHandle>();
        m_resourceManager->CreateVertexLayoutDescription(handle);
        return handle;
    }

    void GFXRenderThread::DestroyImmediate(BufferHandle handle)
    {
        if (!handle.IsValid()) return;
        m_resourceManager->DestroyResource(handle.index, handle.generation);
    }

    void GFXRenderThread::DestroyImmediate(TextureHandle handle)
    {
        if (!handle.IsValid()) return;
        m_resourceManager->DestroyResource(handle.index, handle.generation);
    }

    void GFXRenderThread::DestroyImmediate(FrameBufferObjectHandle handle)
    {
        if (!handle.IsValid()) return;
        m_resourceManager->DestroyResource(handle.index, handle.generation);
    }

    void GFXRenderThread::DestroyImmediate(GpuProgramHandle handle)
    {
        if (!handle.IsValid()) return;
        m_resourceManager->DestroyResource(handle.index, handle.generation);
    }

    void GFXRenderThread::DestroyImmediate(DescriptorSetLayoutHandle handle)
    {
        if (!handle.IsValid()) return;
        m_resourceManager->DestroyResource(handle.index, handle.generation);
    }

    void GFXRenderThread::DestroyImmediate(VertexLayoutDescriptionHandle handle)
    {
        if (!handle.IsValid()) return;
        m_resourceManager->DestroyResource(handle.index, handle.generation);
    }

    void GFXRenderThread::UploadBufferImmediate(BufferHandle handle, const void* data, size_t size)
    {
        if (!handle.IsValid()) return;
        m_resourceManager->UploadBuffer(handle, data, size);
    }

    void GFXRenderThread::UploadTextureImmediate(TextureHandle handle, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format)
    {
        if (!handle.IsValid()) return;
        m_resourceManager->UploadTexture(handle, data, width, height, format);
    }

    // -------------------------------------------------------------------------
    // Flush
    // -------------------------------------------------------------------------
    void GFXRenderThread::Flush()
    {
        std::vector<Command> queue;
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            queue.swap(m_deferredQueue);
        }

        for (auto& cmd : queue)
        {
            cmd(*m_resourceManager);
        }
    }

    // -------------------------------------------------------------------------
    // Frame lifecycle
    // -------------------------------------------------------------------------
    void GFXRenderThread::BeginFrame(uint64_t frameIndex)
    {
        m_resourceManager->BeginFrame(frameIndex);
    }

    void GFXRenderThread::EndFrame(uint64_t frameIndex)
    {
        m_resourceManager->EndFrame(frameIndex);
    }

} // namespace gfx
