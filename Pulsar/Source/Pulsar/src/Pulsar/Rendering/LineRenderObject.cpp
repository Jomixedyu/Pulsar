#include "Application.h"
#include "AssetManager.h"
#include "Assets/StaticMesh.h"

#include <Pulsar/Rendering/LineRenderObject.h>

namespace pulsar
{
    static inline gfx::GFXDescriptorSetLayout_wp MeshDescriptorSetLayout{};

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
        if (MeshDescriptorSetLayout.expired())
        {
            gfx::GFXDescriptorSetLayoutDesc info{
                gfx::GFXDescriptorType::ConstantBuffer,
                gfx::GFXGpuProgramStageFlags::VertexFragment,
                0, kRenderingDescriptorSpace_ModelInfo};
            m_meshDescriptorSetLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(&info, 1);
            MeshDescriptorSetLayout = m_meshDescriptorSetLayout;
        }
        else
        {
            m_meshDescriptorSetLayout = MeshDescriptorSetLayout.lock();
        }

        gfx::GFXBufferDesc perMeshBufferDesc{};
        perMeshBufferDesc.Usage         = gfx::GFXBufferUsage::ConstantBuffer;
        perMeshBufferDesc.StorageType   = gfx::GFXBufferMemoryPosition::VisibleOnDevice;
                perMeshBufferDesc.BufferSize    = sizeof(PerRendererData);
                perMeshBufferDesc.ElementSize   = sizeof(PerRendererData);

        m_meshConstantBuffer = Application::GetGfxApp()->CreateBuffer(perMeshBufferDesc);
        m_meshObjDescriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_meshDescriptorSetLayout);
        m_meshObjDescriptorSet->AddDescriptor("ModelObject", 0)->SetConstantBuffer(m_meshConstantBuffer.get());
        m_meshObjDescriptorSet->Submit();

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
        batch.Elements.resize(1);
        batch.Elements[0].Vertex = m_vertBuffer;
        batch.Elements[0].ModelDescriptor = m_meshObjDescriptorSet;
        batch.State.Topology = gfx::GFXPrimitiveTopology::LineList;
        batch.State.LineWidth = 1.f;
        batch.State.VertexLayouts = {StaticMesh::StaticGetVertexLayout()};
        batch.IsUsedIndices = false;
        batch.IsDepthTestDisabled = true;  // Gizmo 线条始终可见，不被模型遮挡
        batch.Queue = ShaderPassRenderQueueType::Transparency; // 最后绘制，覆盖在所有物体之上
        batch.Material = AssetManager::Get()->LoadAsset<Material>("Engine/Materials/VertexColor");
    }

    void LineRenderObject::OnDestroyResource()
    {
        base::OnDestroyResource();
        m_vertBuffer.reset();
    }

    void LineRenderObject::OnChangedTransform()
    {
        m_meshConstantBuffer->Fill(&m_perModelData);
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