#pragma once
#include <memory>
#include "GFXShaderPass.h"
#include "GFXDescriptorSet.h"
#include "GFXFrameBufferObject.h"
#include "GFXGraphicsPipeline.h"

namespace gfx
{
    class GFXApplication;

    class GFXCommandBuffer
    {
    public:
        GFXCommandBuffer() {}
        GFXCommandBuffer(const GFXCommandBuffer&) = delete;
        GFXCommandBuffer(GFXCommandBuffer&&) = delete;
        virtual ~GFXCommandBuffer() {}
    public:
        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void SetFrameBuffer(GFXFrameBufferObject* framebuffer) = 0;
        virtual void CmdBindGraphicsPipeline(GFXGraphicsPipeline* pipeline) = 0;
        virtual void CmdBindVertexBuffers(const std::vector<GFXBuffer*>& buffers) = 0;
        virtual void CmdBindIndexBuffer(GFXBuffer* buffer) = 0;
        virtual void CmdBindDescriptorSets(GFXDescriptorSet* descriptorSet, GFXGraphicsPipeline* pipeline) = 0;
        virtual void CmdDrawIndexed(size_t indicesCount) = 0;
        virtual void CmdClearColor(float r, float g, float b, float a) = 0;
        virtual void CmdBeginFrameBuffer() = 0;
        virtual void CmdEndFrameBuffer() = 0;
        virtual void CmdSetViewport(float x, float y, float width, float height) = 0;
    public:
        virtual GFXApplication* GetApplication() const = 0;

    };



}