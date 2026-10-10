#include <Pulsar/Rendering/MeshProxy.h>

#include <Pulsar/Application.h>
#include <gfx/GFXResourceRegistry.h>

namespace pulsar::rendering
{
    void MeshProxy::Init(std::vector<SectionData> sections)
    {
        Release();

        auto gfxApp = Application::GetGfxApp();
        auto* registry = gfxApp ? gfxApp->GetResourceRegistry() : nullptr;
        if (!registry)
            return;

        m_vertexBuffers.reserve(sections.size());
        m_indexBuffers.reserve(sections.size());

        for (auto& section : sections)
        {
            gfx::GFXBufferDesc vDesc{};
            vDesc.Usage       = gfx::GFXBufferUsage::Vertex;
            vDesc.StorageType = gfx::GFXBufferMemoryPosition::DeviceLocal;
            vDesc.BufferSize  = section.VertexData.size();
            vDesc.ElementSize = section.VertexStride;
            auto vb = registry->CreateBuffer(vDesc);
            if (!section.VertexData.empty())
                vb->Update(section.VertexData.data());
            m_vertexBuffers.push_back(std::move(vb));

            gfx::GFXBufferDesc iDesc{};
            iDesc.Usage       = gfx::GFXBufferUsage::Indices;
            iDesc.StorageType = gfx::GFXBufferMemoryPosition::DeviceLocal;
            iDesc.BufferSize  = section.IndexData.size();
            iDesc.ElementSize = sizeof(uint32_t);
            auto ib = registry->CreateBuffer(iDesc);
            if (!section.IndexData.empty())
                ib->Update(section.IndexData.data());
            m_indexBuffers.push_back(std::move(ib));
        }

        m_initialized = true;
    }

    void MeshProxy::Release()
    {
        // Member resets only; the registry defers the actual GPU destruction.
        m_vertexBuffers.clear();
        m_indexBuffers.clear();
        m_initialized = false;
    }
}