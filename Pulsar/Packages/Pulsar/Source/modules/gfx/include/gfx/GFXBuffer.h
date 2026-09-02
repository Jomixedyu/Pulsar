#pragma once
#include "GFXResource.h"

namespace gfx
{
    enum class GFXBufferUsage : uint32_t
    {
        None = 0,
        Vertex = 1 << 0,
        Indices = 1 << 1,
        ConstantBuffer = 1 << 2,
        StructuredBuffer = 1 << 3,
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

    class GFXBuffer : public GFXResource
    {
    public:
        explicit GFXBuffer(const GFXBufferDesc& desc)
            : m_desc(desc) {}
        GFXBuffer(const GFXBuffer&) = delete;
        GFXBuffer(GFXBuffer&&) = delete;
        ~GFXBuffer() override = default;
        GFXResourceType GetResourceType() const override { return GFXResourceType::Buffer; }
    public:
        // Uploads data into the buffer. Host-visible memory is mapped and copied directly;
        // device-local memory is uploaded through a transient staging buffer.
        virtual void Update(const void* data) = 0;
        virtual void Release() = 0;
    public:
        virtual size_t GetSize() const = 0;
        virtual bool IsValid() const = 0;
        size_t GetElementCount() const { return m_desc.BufferSize / m_desc.ElementSize; }
        size_t GetElementSize() const { return m_desc.ElementSize; }
        const GFXBufferDesc& GetDesc() const { return m_desc; }
        // True when the buffer memory is device-local (not host-visible).
        bool IsDeviceLocal() const { return m_desc.StorageType == GFXBufferMemoryPosition::DeviceLocal; }
    protected:
        GFXBufferDesc m_desc;
    };
    GFX_DECL_SPTR(GFXBuffer);
}