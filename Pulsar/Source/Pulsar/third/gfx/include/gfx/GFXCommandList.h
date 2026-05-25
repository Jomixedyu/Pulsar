#pragma once
#include "GFXRefCountPtr.h"
#include "GFXResourceManager.h"
#include "GFXCommandBuffer.h"

#include <vector>
#include <mutex>
#include <functional>

namespace gfx
{
    // -------------------------------------------------------------------------
    // Command list abstraction.
    //
    // Provides both deferred (Enqueue/Flush) and synchronous operations.
    // Render commands (CmdDraw, CmdBindGraphicsPipeline, ...) are NOT here;
    // callers obtain the active GFXCommandBuffer via GetCommandBuffer() and
    // issue draw commands directly on it.
    //
    // Resource creation returns GFXRefCountPtr<T>. Destruction is automatic
    // via intrusive reference counting (no explicit Destroy needed).
    // -------------------------------------------------------------------------
    class GFXCommandList
    {
    public:
        using Command = std::function<void(GFXCommandList&)>;

        virtual ~GFXCommandList() = default;

        // -----------------------------------------------------------------
        // Deferred submission
        // -----------------------------------------------------------------
        virtual void Enqueue(Command cmd) = 0;
        virtual void Flush() = 0;

        // -----------------------------------------------------------------
        // Frame lifecycle (forwarded to resource manager)
        // -----------------------------------------------------------------
        virtual void BeginFrame(uint64_t frameIndex) = 0;
        virtual void EndFrame(uint64_t frameIndex) = 0;

        // -----------------------------------------------------------------
        // Resource creation (synchronous — caller gets ref-counted pointer)
        // -----------------------------------------------------------------
        virtual GFXRefCountPtr<GFXBuffer> CreateBuffer(const GFXBufferDesc& desc) = 0;
        virtual GFXRefCountPtr<GFXTexture> CreateTexture2D(const GFXTextureCreateDesc& desc) = 0;
        virtual GFXRefCountPtr<GFXTexture> CreateTextureCube(int32_t size) = 0;
        virtual GFXRefCountPtr<GFXTexture> CreateRenderTarget(const GFXTextureCreateDesc& desc) = 0;
        virtual GFXRefCountPtr<GFXFrameBufferObject> CreateFrameBufferObject(const array_list<GFXTexture2DView_sp>& attachments) = 0;
        virtual GFXRefCountPtr<GFXGpuProgram> CreateGpuProgram(GFXGpuProgramStageFlags stage, const void* code, size_t length) = 0;
        virtual GFXRefCountPtr<GFXDescriptorSetLayout> CreateDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutDesc>& bindings) = 0;
        virtual GFXRefCountPtr<GFXVertexLayoutDescription> CreateVertexLayoutDescription() = 0;

        // -----------------------------------------------------------------
        // Resource upload (caller keeps resource alive via GFXRefCountPtr)
        // -----------------------------------------------------------------
        virtual void UploadBuffer(GFXBuffer* buffer, const void* data, size_t size) = 0;
        virtual void UploadTexture(GFXTexture* texture, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format) = 0;

        // -----------------------------------------------------------------
        // Active command buffer (for render commands)
        // -----------------------------------------------------------------
        GFXCommandBuffer* GetCommandBuffer() const { return m_commandBuffer; }
        void SetCommandBuffer(GFXCommandBuffer* cmdBuffer) { m_commandBuffer = cmdBuffer; }

    protected:
        GFXResourceManager* m_resourceManager = nullptr;
        GFXCommandBuffer*   m_commandBuffer   = nullptr;
    };

    // -------------------------------------------------------------------------
    // Immediate command list — Enqueue() executes synchronously on the caller
    // thread.  Flush() is a no-op.
    // -------------------------------------------------------------------------
    class GFXCommandListImmediate final : public GFXCommandList
    {
    public:
        explicit GFXCommandListImmediate(GFXResourceManager* resMgr);

        void Enqueue(Command cmd) override;
        void Flush() override;
        void BeginFrame(uint64_t frameIndex) override;
        void EndFrame(uint64_t frameIndex) override;

        GFXRefCountPtr<GFXBuffer> CreateBuffer(const GFXBufferDesc& desc) override;
        GFXRefCountPtr<GFXTexture> CreateTexture2D(const GFXTextureCreateDesc& desc) override;
        GFXRefCountPtr<GFXTexture> CreateTextureCube(int32_t size) override;
        GFXRefCountPtr<GFXTexture> CreateRenderTarget(const GFXTextureCreateDesc& desc) override;
        GFXRefCountPtr<GFXFrameBufferObject> CreateFrameBufferObject(const array_list<GFXTexture2DView_sp>& attachments) override;
        GFXRefCountPtr<GFXGpuProgram> CreateGpuProgram(GFXGpuProgramStageFlags stage, const void* code, size_t length) override;
        GFXRefCountPtr<GFXDescriptorSetLayout> CreateDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutDesc>& bindings) override;
        GFXRefCountPtr<GFXVertexLayoutDescription> CreateVertexLayoutDescription() override;

        void UploadBuffer(GFXBuffer* buffer, const void* data, size_t size) override;
        void UploadTexture(GFXTexture* texture, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format) override;
    };

    // -------------------------------------------------------------------------
    // Deferred command list — Enqueue() stores lambdas in an internal queue.
    // Flush() drains the queue and executes them on the caller thread.
    //
    // Creation/Upload methods are synchronous (they touch the resource
    // manager directly) so that callers can use resources immediately.
    // -------------------------------------------------------------------------
    class GFXCommandListDeferred final : public GFXCommandList
    {
    public:
        explicit GFXCommandListDeferred(GFXResourceManager* resMgr);

        void Enqueue(Command cmd) override;
        void Flush() override;
        void BeginFrame(uint64_t frameIndex) override;
        void EndFrame(uint64_t frameIndex) override;

        GFXRefCountPtr<GFXBuffer> CreateBuffer(const GFXBufferDesc& desc) override;
        GFXRefCountPtr<GFXTexture> CreateTexture2D(const GFXTextureCreateDesc& desc) override;
        GFXRefCountPtr<GFXTexture> CreateTextureCube(int32_t size) override;
        GFXRefCountPtr<GFXTexture> CreateRenderTarget(const GFXTextureCreateDesc& desc) override;
        GFXRefCountPtr<GFXFrameBufferObject> CreateFrameBufferObject(const array_list<GFXTexture2DView_sp>& attachments) override;
        GFXRefCountPtr<GFXGpuProgram> CreateGpuProgram(GFXGpuProgramStageFlags stage, const void* code, size_t length) override;
        GFXRefCountPtr<GFXDescriptorSetLayout> CreateDescriptorSetLayout(const std::vector<GFXDescriptorSetLayoutDesc>& bindings) override;
        GFXRefCountPtr<GFXVertexLayoutDescription> CreateVertexLayoutDescription() override;

        void UploadBuffer(GFXBuffer* buffer, const void* data, size_t size) override;
        void UploadTexture(GFXTexture* texture, const void* data, uint32_t width, uint32_t height, GFXTextureFormat format) override;

    private:
        std::vector<Command> m_queue;
        std::mutex m_mutex;
    };

} // namespace gfx
