#pragma once

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
            : m_usage(usage), m_bufferSize(bufferSize) {}
        GFXBuffer(const GFXBuffer&) = delete;
        GFXBuffer(GFXBuffer&&) = delete;
        virtual ~GFXBuffer() {}
    public:
        virtual void Fill(const void* data) = 0;
        virtual void Release() = 0;
    public:
        virtual size_t GetSize() const = 0;
        virtual bool IsValid() const = 0;
    protected:
        GFXBufferUsage m_usage;
        size_t m_bufferSize;
    };

}