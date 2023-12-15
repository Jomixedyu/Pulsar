#include "Application.h"
#include "AssetManager.h"

#include <Pulsar/Rendering/LineRenderObject.h>

namespace pulsar
{
    static inline gfx::GFXDescriptorSetLayout_wp MeshDescriptorSetLayout{};

    void LineRenderObject::SetPoints(const array_list<Vector3f>& pointPairs, const array_list<Color4f>& pointColors)
    {
        for (size_t i = 0; i < pointPairs.size(); i++)
        {
            StaticMeshVertex vert{};
            vert.Color = pointColors[i];
            vert.Position = pointPairs[i];

            m_verties.push_back(vert);
        }
    }
    void LineRenderObject::SetVerties(const array_list<StaticMeshVertex>& verties)
    {
        m_verties = verties;
    }

    void LineRenderObject::OnCreateResource()
    {
        base::OnCreateResource();
        if (MeshDescriptorSetLayout.expired())
        {
            gfx::GFXDescriptorSetLayoutInfo info{
                gfx::GFXDescriptorType::ConstantBuffer,
                gfx::GFXShaderStageFlags::VertexFragment,
                0, kRenderingDescriptorSpace_ModelInfo};
            m_meshDescriptorSetLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(&info, 1);
            MeshDescriptorSetLayout = m_meshDescriptorSetLayout;
        }
        else
        {
            m_meshDescriptorSetLayout = MeshDescriptorSetLayout.lock();
        }

        m_meshConstantBuffer = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::ConstantBuffer, sizeof(CBuffer_ModelObject));
        m_meshObjDescriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_meshDescriptorSetLayout);
        m_meshObjDescriptorSet->AddDescriptor("ModelObject", 0)->SetConstantBuffer(m_meshConstantBuffer.get());
        m_meshObjDescriptorSet->Submit();

        m_vertBuffer = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::Vertex, m_verties.size() * sizeof(StaticMeshVertex));
        m_vertBuffer->Fill(m_verties.data());

        m_batchs.resize(1);
        rendering::MeshBatch& batch = m_batchs[0];
        batch.DescriptorSetLayout = m_meshDescriptorSetLayout;
        batch.Elements.resize(1);
        batch.Elements[0].Vertex = m_vertBuffer;
        batch.Elements[0].ModelDescriptor = m_meshObjDescriptorSet;
        batch.State.Topology = gfx::GFXPrimitiveTopology::LineList;
        batch.State.LineWidth = 1.f;
        batch.IsUsedIndices = false;
        batch.Material = GetAssetManager()->LoadAsset<Material>("Engine/Materials/VertexColor");
    }
} // namespace pulsar