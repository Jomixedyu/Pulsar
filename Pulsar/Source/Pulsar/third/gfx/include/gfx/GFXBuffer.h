#pragma once
#include "GFXInclude.h"

namespace gfx
{
    enum class GFXBufferUsage
    {
        Vertex,
        Indices,
        ConstantBuffer,
        StructuredBuffer,
    };

    enum class GFXBufferMemoryPosition
    {
        VisibleOnHost,
        VisibleOnDevice,
        DeviceLocal,
    };

    struct GFXBufferDesc
    {
        GFXBufferUsage Usage;
        GFXBufferMemoryPosition StorageType;
        size_t BufferSize;
        size_t ElementSize;
    };

    class GFXBuffer
    {
    public:
        explicit GFXBuffer(const GFXBufferDesc& desc)
            : m_desc(desc) {}
        GFXBuffer(const GFXBuffer&) = delete;
        GFXBuffer(GFXBuffer&&) = delete;
        virtual ~GFXBuffer() = default;
    public:
        virtual void Fill(const void* data) = 0;
        virtual void Release() = 0;
    public:
        virtual size_t GetSize() const = 0;
        virtual bool IsValid() const = 0;
        size_t GetElementCount() const { return m_desc.BufferSize / m_desc.ElementSize; }
        size_t GetElementSize() const { return m_desc.ElementSize; }
        const GFXBufferDesc& GetDesc() const { return m_desc; }
    protected:
        GFXBufferDesc m_desc;
    };
    GFX_DECL_SPTR(GFXBuffer);
}