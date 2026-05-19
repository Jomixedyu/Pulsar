#include "Application.h"
#include "AssetManager.h"
#include "Assets/StaticMesh.h"

#include <Pulsar/Rendering/LineRenderObject.h>

namespace pulsar
{
    // Layout for set2 (dummy, kept for pipeline compatibility)
    static inline gfx::GFXDescriptorSetLayout_wp s_dummyLayout{};

    void LineRenderObject::SetPoints(const array_list<Vector3f>& pointPairs, const array_list<Color4b>& pointColors)
    {
        m_verties.clear();
        for (size_t i = 0; i < pointPairs.size(); i++)
        {
            StaticMeshVertex vert{};
            vert.Color = pointColors[i];
            vert.Position = pointPairs[i];

            m_verties.push_back(vert);
        }
        Fill();
    }

    void LineRenderObject::SetVerties(const array_list<StaticMeshVertex>& verties)
    {
        m_verties = verties;
        Fill();
    }
    void LineRenderObject::Fill()
    {
        if (m_vertBuffer)
        {
            if (sizeof(StaticMeshVertex) * m_verties.size() > m_vertBuffer->GetSize())
            {
                gfx::GFXBufferDesc desc{};
                desc.Usage        = gfx::GFXBufferUsage::Vertex;
                desc.StorageType  = gfx::GFXBufferMemoryPosition::DeviceLocal;
                desc.BufferSize   = m_verties.size() * sizeof(StaticMeshVertex);
                desc.ElementSize  = sizeof(StaticMeshVertex);

                m_vertBuffer = Application::GetGfxApp()->CreateBuffer(desc);
            }
            m_vertBuffer->Fill(m_verties.data());
        }
    }

    void LineRenderObject::OnCreateResource()
    {
        base::OnCreateResource();
        if (m_pPerRenderObjectDataManager)
            m_dummyExtraSet = m_pPerRenderObjectDataManager->GetDummyExtraSet();

        if (s_dummyLayout.expired())
        {
            gfx::GFXDescriptorSetLayoutDesc info{};
            m_meshDescriptorSetLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(&info, 0);
            s_dummyLayout = m_meshDescriptorSetLayout;
        }
        else
        {
            m_meshDescriptorSetLayout = s_dummyLayout.lock();
        }

        gfx::GFXBufferDesc vertexBufferDesc{};
        vertexBufferDesc.Usage       = gfx::GFXBufferUsage::Vertex;
        vertexBufferDesc.StorageType = gfx::GFXBufferMemoryPosition::DeviceLocal;
        vertexBufferDesc.BufferSize  = m_verties.size() * sizeof(StaticMeshVertex);
        vertexBufferDesc.ElementSize = sizeof(StaticMeshVertex);

        m_vertBuffer = Application::GetGfxApp()->CreateBuffer(vertexBufferDesc);
        m_vertBuffer->Fill(m_verties.data());

        m_batchs.resize(1);
        rendering::MeshBatch& batch = m_batchs[0];
        batch.Interface = GetInterface();
        batch.DescriptorSetLayout = m_meshDescriptorSetLayout;
        batch.RenderObjectIndex = m_renderObjectIndex;
        batch.ExtraDescriptorSet = m_dummyExtraSet;
        batch.Elements.resize(1);
        batch.Elements[0].Vertex = m_vertBuffer;
        batch.State.Topology = gfx::GFXPrimitiveTopology::LineList;
        batch.State.LineWidth = 1.f;
        batch.State.VertexLayouts = {StaticMesh::StaticGetVertexLayout()};
        batch.IsUsedIndices = false;
        batch.IsDepthTestDisabled = !m_depthTestEnabled;
        batch.Material = AssetManager::Get()->LoadAsset<Material>("Engine/Materials/VertexColor");
        if (batch.Material)
            batch.Material->SetQueue(m_renderQueue);
    }

    void LineRenderObject::OnDestroyResource()
    {
        base::OnDestroyResource();
        m_vertBuffer.reset();
    }

    void LineRenderObject::OnChangedTransform()
    {
    }

    array_list<rendering::MeshBatch> LineRenderObject::GetMeshBatches()
    {
        for (const auto& batch : m_batchs)
        {
            if (batch.Material && !batch.Material->IsCreatedGPUResource())
            {
                batch.Material->CreateGPUResource();
            }
        }
        return m_batchs;
    }
} // namespace pulsar