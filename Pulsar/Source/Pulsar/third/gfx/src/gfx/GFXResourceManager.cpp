#include "gfx/GFXResourceManager.h"
#include "gfx/GFXApplication.h"
#include "gfx/GFXDeferredDestroyQueue.h"

namespace gfx
{
    GFXResourceManager::GFXResourceManager(GFXApplication* app)
        : m_app(app)
    {
    }

    GFXResourceManager::~GFXResourceManager()
    {
        // Process any remaining deferred destroys immediately
        GFXDeferredDestroyQueue::AdvanceFrame();
        GFXDeferredDestroyQueue::AdvanceFrame();
        GFXDeferredDestroyQueue::AdvanceFrame();
    }

    // -------------------------------------------------------------------------
    // Factory methods
    // -------------------------------------------------------------------------
    GFXRefCountPtr<GFXBuffer> GFXResourceManager::CreateBuffer(const GFXBufferDesc& desc)
    {
        return m_app->CreateBuffer(desc);
    }

    GFXRefCountPtr<GFXTexture> GFXResourceManager::CreateTexture2D(const GFXTextureCreateDesc& desc)
    {
        return m_app->CreateTexture2DFromMemory(
            desc.ImageData, desc.DataLength,
            desc.Width, desc.Height,
            desc.Format, desc.SamplerCfg);
    }

    GFXRefCountPtr<GFXTexture> GFXResourceManager::CreateTextureCube(int32_t size)
    {
        return m_app->CreateTextureCube(size);
    }

    GFXRefCountPtr<GFXTexture> GFXResourceManager::CreateRenderTarget(const GFXTextureCreateDesc& desc)
    {
        return m_app->CreateRenderTarget(
            desc.Width, desc.Height, desc.TargetType,
            desc.Format, desc.SamplerCfg,
            desc.SampleCount, desc.IsTransientAttachment);
    }

    GFXRefCountPtr<GFXFrameBufferObject> GFXResourceManager::CreateFrameBufferObject(const array_list<GFXTexture2DView_sp>& attachments)
    {
        return m_app->CreateFrameBufferObject(attachments);
    }

    GFXRefCountPtr<GFXGpuProgram> GFXResourceManager::CreateGpuProgram(GFXGpuProgramStageFlags stage, const void* code, size_t length)
    {
        return m_app->CreateGpuProgram(stage, static_cast<const uint8_t*>(code), length);
    }

    GFXRefCountPtr<GFXDescriptorSetLayout> GFXResourceManager::CreateDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutDesc>& bindings)
    {
        return m_app->CreateDescriptorSetLayout(bindings.data(), bindings.size());
    }

    GFXRefCountPtr<GFXVertexLayoutDescription> GFXResourceManager::CreateVertexLayoutDescription()
    {
        return m_app->CreateVertexLayoutDescription();
    }

    // -------------------------------------------------------------------------
    // Resource upload
    // -------------------------------------------------------------------------
    void GFXResourceManager::UploadBuffer(GFXBuffer* buffer, const void* data, size_t /*size*/)
    {
        if (buffer)
        {
            buffer->Fill(data);
        }
    }

    void GFXResourceManager::UploadTexture(GFXTexture* /*texture*/, const void* /*data*/, uint32_t /*width*/, uint32_t /*height*/, GFXTextureFormat /*format*/)
    {
        // TODO: implement texture upload via gfx app
    }

    // -------------------------------------------------------------------------
    // Frame lifecycle
    // -------------------------------------------------------------------------
    void GFXResourceManager::BeginFrame(uint64_t frameIndex)
    {
        m_currentFrame = frameIndex;
        GFXDeferredDestroyQueue::AdvanceFrame();
    }

    void GFXResourceManager::EndFrame(uint64_t frameIndex)
    {
        m_currentFrame = frameIndex;
        // AdvanceFrame is called only in BeginFrame to maintain a consistent
        // 2-frame deferred destruction window. Calling it in both places
        // halves the effective delay and can cause GPU-in-use resources to
        // be destroyed prematurely.
    }

} // namespace gfx
