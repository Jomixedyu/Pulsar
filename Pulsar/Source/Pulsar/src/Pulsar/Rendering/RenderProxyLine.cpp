#include "Application.h"
#include "AssetManager.h"
#include "Assets/StaticMesh.h"
#include <Pulsar/Rendering/PerRenderObjectDataManager.h>
#include <Pulsar/Rendering/RenderProxyMaterial.h>

#include <Pulsar/Rendering/RenderProxyLine.h>

namespace pulsar
{
    // Layout for set2 (dummy, kept for pipeline compatibility)
    static inline gfx::GFXDescriptorSetLayout_wp s_dummyLayout{};

    void RenderProxyLine::SetPoints(const array_list<Vector3f>& pointPairs, const array_list<Color4b>& pointColors)
    {
        m_verties.clear();
        for (size_t i = 0; i < pointPairs.size(); i++)
        {
            StaticMeshVertex vert{};
            vert.Color = pointColors[i];
            vert.Position = pointPairs[i];

            m_verties.push_back(vert);
        }
        auto* proxy = this;
        RenderThread::Get().EnqueueCommand([proxy]() {
            proxy->Fill();
        });
    }

    void RenderProxyLine::SetVerties(const array_list<StaticMeshVertex>& verties)
    {
        m_verties = verties;
        auto* proxy = this;
        RenderThread::Get().EnqueueCommand([proxy]() {
            proxy->Fill();
        });
    }
    void RenderProxyLine::Fill()
    {
        auto& cmdList = Application::GetGfxApp()->GetImmediateCommandList();
        auto* resMgr  = Application::GetGfxApp()->GetResourceManager();

        if (m_vertBuffer.IsValid())
        {
            if (sizeof(StaticMeshVertex) * m_verties.size() > m_vertBuffer->GetSize())
            {
                m_vertBuffer.Reset();
            }
        }

        if (!m_vertBuffer.IsValid() && !m_verties.empty())
        {
            gfx::GFXBufferDesc desc{};
            desc.Usage        = gfx::GFXBufferUsage::Vertex;
            desc.StorageType  = gfx::GFXBufferMemoryPosition::DeviceLocal;
            desc.BufferSize   = m_verties.size() * sizeof(StaticMeshVertex);
            desc.ElementSize  = sizeof(StaticMeshVertex);

            m_vertBuffer = cmdList.CreateBuffer(desc);
        }

        if (m_vertBuffer.IsValid() && !m_verties.empty())
        {
            cmdList.UploadBuffer(m_vertBuffer.Get(), m_verties.data(), m_verties.size() * sizeof(StaticMeshVertex));
        }
    }

    void RenderProxyLine::InitRHI()
    {
        base::InitRHI();
        m_dummyExtraSet = RenderThread::Get().GetPerObjectDataManager().GetDummyExtraSet();

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

            auto& cmdList = Application::GetGfxApp()->GetImmediateCommandList();
            m_vertBuffer = cmdList.CreateBuffer(vertexBufferDesc);
            cmdList.UploadBuffer(m_vertBuffer.Get(), m_verties.data(), m_verties.size() * sizeof(StaticMeshVertex));
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
        batch.Material = AssetManager::Get()->LoadAsset<Material>("Engine/Materials/VertexColor");
        if (batch.Material)
        {
            batch.Material->SetQueue(m_renderQueue);
            batch.ProxyMaterial = mksptr(new RenderProxyMaterial(batch.Material));
        }
    }

    void RenderProxyLine::ReleaseRHI()
    {
        base::ReleaseRHI();
        m_vertBuffer.Reset();
    }

    void RenderProxyLine::OnChangedTransform()
    {
    }

    array_list<rendering::MeshBatch> RenderProxyLine::GetMeshBatches()
    {
        return m_batchs;
    }
} // namespace pulsar