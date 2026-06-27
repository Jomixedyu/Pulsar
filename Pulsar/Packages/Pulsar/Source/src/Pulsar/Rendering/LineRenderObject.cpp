#include "Application.h"
#include "AssetManager.h"
#include "Assets/StaticMesh.h"

#include <Pulsar/Rendering/LineRenderObject.h>
#include <gfx/GFXResourceManager.h>

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
        auto* resMgr  = Application::GetGfxApp()->GetResourceManager();

        if (m_vertBuffer.IsValid())
        {
            auto* buffer = resMgr->GetBuffer(m_vertBuffer);
            if (buffer && sizeof(StaticMeshVertex) * m_verties.size() > buffer->GetSize())
            {
                resMgr->Destroy(m_vertBuffer);
                m_vertBuffer = gfx::BufferHandle{};
            }
        }

        if (!m_vertBuffer.IsValid() && !m_verties.empty())
        {
            gfx::GFXBufferDesc desc{};
            desc.Usage        = gfx::GFXBufferUsage::Vertex;
            desc.StorageType  = gfx::GFXBufferMemoryPosition::DeviceLocal;
            desc.BufferSize   = m_verties.size() * sizeof(StaticMeshVertex);
            desc.ElementSize  = sizeof(StaticMeshVertex);

            m_vertBuffer = resMgr->AllocHandle<gfx::BufferHandle>();
            resMgr->CreateBuffer(m_vertBuffer, desc);
        }

        if (m_vertBuffer.IsValid() && !m_verties.empty())
        {
            resMgr->UploadBuffer(m_vertBuffer, m_verties.data(), m_verties.size() * sizeof(StaticMeshVertex));
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

        if (!m_vertBuffer.IsValid() && !m_verties.empty())
        {
            gfx::GFXBufferDesc vertexBufferDesc{};
            vertexBufferDesc.Usage       = gfx::GFXBufferUsage::Vertex;
            vertexBufferDesc.StorageType = gfx::GFXBufferMemoryPosition::DeviceLocal;
            vertexBufferDesc.BufferSize  = m_verties.size() * sizeof(StaticMeshVertex);
            vertexBufferDesc.ElementSize = sizeof(StaticMeshVertex);

            auto* resMgr = Application::GetGfxApp()->GetResourceManager();
            m_vertBuffer = resMgr->AllocHandle<gfx::BufferHandle>();
            resMgr->CreateBuffer(m_vertBuffer, vertexBufferDesc);
            resMgr->UploadBuffer(m_vertBuffer, m_verties.data(), m_verties.size() * sizeof(StaticMeshVertex));
        }

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
        m_material = AssetManager::Get()->LoadAsset<Material>("Pulsar/Materials/VertexColor");
        if (m_material)
            m_material->SetQueue(m_renderQueue);
    }

    void LineRenderObject::OnDestroyResource()
    {
        base::OnDestroyResource();
        if (m_vertBuffer.IsValid())
        {
            auto* resMgr = Application::GetGfxApp()->GetResourceManager();
            resMgr->Destroy(m_vertBuffer);
            m_vertBuffer = gfx::BufferHandle{};
        }
    }

    void LineRenderObject::OnChangedTransform()
    {
    }

    array_list<rendering::MeshBatch> LineRenderObject::GetMeshBatches()
    {
        if (m_material && !m_material->IsCreatedGPUResource())
            m_material->CreateGPUResource();

        for (auto& batch : m_batchs)
        {
            batch.Material = m_material ? m_material->GetRenderProxy() : nullptr;
        }
        return m_batchs;
    }
} // namespace pulsar