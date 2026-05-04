#pragma once
#include "GFXDescriptorSet.h"
#include "GFXFrameBufferObject.h"
#include "GFXGraphicsPipeline.h"
#include "GFXInclude.h"
#include "GFXRenderPass.h"
#include "GFXTextureView.h"
#include <memory>

namespace gfx
{
    class GFXApplication;

    enum class GFXResourceLayout
    {
        RenderTarget,
        ShaderReadOnly
    };

    class GFXCommandBuffer
    {
    public:
        GFXCommandBuffer() = default;
        GFXCommandBuffer(const GFXCommandBuffer&) = delete;
        GFXCommandBuffer(GFXCommandBuffer&&) = delete;
        virtual ~GFXCommandBuffer() = default;
    public:
        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void SetFrameBuffer(GFXFrameBufferObject* framebuffer) = 0;
        virtual void CmdBindGraphicsPipeline(GFXGraphicsPipeline* pipeline) = 0;
        virtual void CmdBindVertexBuffers(const std::vector<GFXBuffer*>& buffers) = 0;
        virtual void CmdBindIndexBuffer(GFXBuffer* buffer) = 0;
        virtual void CmdBindDescriptorSets(const array_list<GFXDescriptorSet*>& descriptorSet, GFXGraphicsPipeline* pipeline) = 0;

        virtual void CmdPushDebugInfo(std::string_view label, const std::array<float, 4>& color = {}) = 0;
        virtual void CmdPopDebugInfo() = 0;

        virtual void CmdDraw(size_t vertexCount) = 0;
        virtual void CmdDrawIndexed(size_t indicesCount) = 0;
        virtual void CmdClearColor(GFXTexture* rt, float r, float g, float b, float a) = 0;
        virtual void CmdClearColor(GFXTexture* rt) = 0;

        virtual void CmdBeginRenderPass(std::string_view name = "",
                                        const GFXRenderPassBeginInfo& info = {}) = 0;
        virtual void CmdEndRenderPass() = 0;
        virtual void CmdSetViewport(float x, float y, float width, float height) = 0;
        virtual void CmdSetCullMode(GFXCullMode mode) = 0;
        virtual void CmdBlit(GFXTextureView* src, GFXTextureView* dest) = 0;
        virtual void CmdResolve(GFXTextureView* src, GFXTextureView* dest) = 0;
        virtual GFXFrameBufferObject* GetFrameBuffer() const = 0;

        virtual void CmdImageTransitionBarrier(GFXTextureView* rt, GFXResourceLayout layout) = 0;
    public:
        virtual GFXApplication* GetApplication() const = 0;

    };
    GFX_DECL_SPTR(GFXCommandBuffer);

    struct GFXDebugEventScoped
    {
        gfx::GFXCommandBuffer& m_buffer;
        GFXDebugEventScoped(gfx::GFXCommandBuffer& buffer, std::string_view label) : m_buffer(buffer)
        {
            m_buffer.CmdPushDebugInfo(label.data());
        }
        ~GFXDebugEventScoped()
        {
            m_buffer.CmdPopDebugInfo();
        }
    };

}