#pragma once
#include "GFXInclude.h"

namespace gfx
{
    enum class GFXBufferUsage
    {
        Vertex,
        Index,
        ConstantBuffer,
    };
    enum class GFXBufferStorageState
    {
        Default,
        Staging,
        GpuLocal,
    };
    class GFXBuffer
    {
    public:
        GFXBuffer(GFXBufferUsage usage, size_t bufferSize)
            : m_usage(usage), m_bufferSize(bufferSize), m_elementCount(1) {}
        GFXBuffer(const GFXBuffer&) = delete;
        GFXBuffer(GFXBuffer&&) = delete;
        virtual ~GFXBuffer() = default;
    public:
        virtual void Fill(const void* data) = 0;
        virtual void Release() = 0;
    public:
        virtual size_t GetSize() const = 0;
        virtual bool IsValid() const = 0;
        size_t GetElementCount() const { return m_elementCount; }
        void SetElementCount(size_t itemCount) { m_elementCount = itemCount; }
        size_t GetElementSize() const { return m_bufferSize / m_elementCount; }
    protected:
        GFXBufferUsage m_usage;
        size_t m_bufferSize;
        size_t m_elementCount;
    };
    GFX_DECL_SPTR(GFXBuffer);
}