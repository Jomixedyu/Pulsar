#pragma once
#include "GFXRefCountPtr.h"
#include "GFXResource.h"
#include "GFXBuffer.h"
#include "GFXTexture.h"
#include "GFXGpuProgram.h"
#include "GFXCommandBuffer.h"

#include <vector>

namespace gfx
{
    class GFXApplication;

    // -------------------------------------------------------------------------
    // Resource manager: factory for GPU resources.
    //
    // In the new architecture, resources are owned by GFXRefCountPtr<T>
    // (intrusive reference counting). The manager no longer maintains a
    // slot table; it simply forwards creation requests to GFXApplication.
    //
    // Destruction is deferred by GFXDeferredDestroyQueue (2 frames) to
    // ensure the GPU is no longer using the resource.
    // -------------------------------------------------------------------------
    class GFXResourceManager
    {
    public:
        explicit GFXResourceManager(GFXApplication* app);
        ~GFXResourceManager();

        // -----------------------------------------------------------------
        // Factory methods (synchronous — caller gets ref-counted pointer)
        // -----------------------------------------------------------------
        GFXRefCountPtr<GFXBuffer> CreateBuffer(const GFXBufferDesc& desc);
        GFXRefCountPtr<GFXTexture> CreateTexture2D(const GFXTextureCreateDesc& desc);
        GFXRefCountPtr<GFXTexture> CreateTextureCube(int32_t size);
        GFXRefCountPtr<GFXTexture> CreateRenderTarget(const GFXTextureCreateDesc& desc);
        GFXRefCountPtr<GFXFrameBufferObject> CreateFrameBufferObject(const array_list<GFXTexture2DView_sp>& attachments);
        GFXRefCountPtr<GFXGpuProgram> CreateGpuProgram(GFXGpuProgramStageFlags stage, const void* code, size_t length);
        GFXRefCountPtr<GFXDescriptorSetLayout> CreateDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutDesc>& bindings);
        GFXRefCountPtr<GFXVertexLayoutDescription> CreateVertexLayoutDescription();

        // -----------------------------------------------------------------
        // Resource upload (operates on raw pointers — the caller keeps
        // the resource alive via GFXRefCountPtr)
        // -----------------------------------------------------------------
        void UploadBuffer(GFXBuffer* buffer, const void* data, size_t size);
        void UploadTexture(GFXTexture* texture, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format);

        // -----------------------------------------------------------------
        // Frame lifecycle
        // -----------------------------------------------------------------
        void BeginFrame(uint64_t frameIndex);
        void EndFrame(uint64_t frameIndex);

    private:
        GFXApplication* m_app = nullptr;
        uint64_t m_currentFrame = 0;
    };

} // namespace gfx
