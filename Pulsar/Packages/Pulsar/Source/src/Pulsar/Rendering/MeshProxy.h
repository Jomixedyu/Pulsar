#pragma once

#include <gfx/GFXBuffer.h>

#include <cstdint>
#include <cstring>
#include <vector>

namespace pulsar::rendering
{
    // Render-side mirror of a mesh asset's GPU buffers.
    // GPUResourceProxy convention: Init()/Release() run on the render thread
    // (via the render queue); Init is re-init safe. The mesh asset only holds
    // this proxy; GFX handles never touch the asset.
    class MeshProxy final
    {
    public:
        struct SectionData
        {
            std::vector<uint8_t> VertexData;
            uint32_t             VertexStride = 0;
            std::vector<uint8_t> IndexData;
        };

        MeshProxy() = default;
        ~MeshProxy() { Release(); }

        void Init(std::vector<SectionData> sections);
        void Release();

        bool IsInitialized() const { return m_initialized; }

        const std::vector<gfx::GFXBufferPtr>& GetVertexBuffers() const { return m_vertexBuffers; }
        const std::vector<gfx::GFXBufferPtr>& GetIndexBuffers() const { return m_indexBuffers; }

    private:
        std::vector<gfx::GFXBufferPtr> m_vertexBuffers;
        std::vector<gfx::GFXBufferPtr> m_indexBuffers;
        bool m_initialized = false;
    };

    // Builds upload-ready section data from a mesh asset's serialization sections.
    template <typename TSection, typename TVertex>
    std::vector<MeshProxy::SectionData> BuildMeshProxySections(const std::vector<TSection>& sections)
    {
        std::vector<MeshProxy::SectionData> result;
        result.reserve(sections.size());
        for (auto& section : sections)
        {
            auto verts = section.BuildInterleavedVertices();

            MeshProxy::SectionData data;
            data.VertexStride = sizeof(TVertex);
            data.VertexData.resize(verts.size() * sizeof(TVertex));
            std::memcpy(data.VertexData.data(), verts.data(), data.VertexData.size());
            data.IndexData.resize(section.GetIndicesAllocSize());
            std::memcpy(data.IndexData.data(), section.Indices.data(), data.IndexData.size());
            result.push_back(std::move(data));
        }
        return result;
    }
}