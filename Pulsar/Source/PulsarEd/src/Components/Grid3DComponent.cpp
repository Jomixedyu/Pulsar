#include "Components/Grid3DComponent.h"

#include "Pulsar/Components/StaticMeshRendererComponent.h"

#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/StaticMesh.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <PulsarEd/Windows/EditorWindowManager.h>
#include <PulsarEd/Windows/SceneWindow.h>
#include <gfx/GFXShaderPass.h>

namespace pulsared
{

    class LineRenderObject : public rendering::RenderObject
    {
        using base = rendering::RenderObject;
    public:
        array_list<Vector3f> m_vert;
        array_list<Color4f> m_colors;
        array_list<StaticMeshVertex> m_verties;

        gfx::GFXBuffer_sp m_vertBuffer;
        gfx::GFXBuffer_sp m_meshConstantBuffer;
        gfx::GFXDescriptorSet_sp m_meshObjDescriptorSet;
        gfx::GFXDescriptorSetLayout_sp m_meshDescriptorSetLayout;

        array_list<rendering::MeshBatch> m_batchs;
    public:
        LineRenderObject() = default;

        static inline gfx::GFXDescriptorSetLayout_wp MeshDescriptorSetLayout{};
        void OnCreateResource() override
        {
            base::OnCreateResource();
            if(MeshDescriptorSetLayout.expired())
            {
                gfx::GFXDescriptorSetLayoutInfo info{
                    gfx::GFXDescriptorType::ConstantBuffer,
                    gfx::GFXShaderStageFlags::VertexFragment,
                    0, kRenderingDescriptorSpace_ModelInfo
                };
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


            for (size_t i = 0; i < m_vert.size(); i++)
            {
                StaticMeshVertex vert{};
                vert.Color = m_colors[i];
                vert.Position = m_vert[i];

                m_verties.push_back(vert);
            }

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
        void OnDestroyResource() override
        {
            base::OnDestroyResource();
            m_vertBuffer.reset();
        }
        void OnChangedTransform() override
        {
            m_meshConstantBuffer->Fill(&m_localToWorld);
        }
        array_list<rendering::MeshBatch> GetMeshBatchs() override
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
    };


    void Grid3DComponent::BeginComponent()
    {
        base::BeginComponent();

        int line_count = 20;
        float detail_distance = 1;
        float total_width = detail_distance * line_count;


        Color4f detailLineColor = { 0.2f, 0.2f, 0.2f, 1 };

        for (int x = -line_count / 2; x <= line_count / 2; x++)
        {
            m_vert.emplace_back( total_width / 2, 0, detail_distance * x );
            m_vert.emplace_back( -total_width / 2, 0 , detail_distance * x );

            Color4f color = detailLineColor;
            if (x == 0)
            {
                color.r = 0.9f;
            }
            m_colors.push_back(color);
            m_colors.push_back(color);
        }
        for (int z = -line_count / 2; z <= line_count / 2; z++)
        {
            m_vert.emplace_back( detail_distance * z, 0, total_width / 2 );
            m_vert.emplace_back( detail_distance * z, 0, -total_width / 2 );
            Color4f color = detailLineColor;
            if (z == 0)
            {
                color.r = 0.2f;
                color.g = 0.2f;
                color.b = 1.f;
            }
            m_colors.push_back(color);
            m_colors.push_back(color);
        }

        m_renderObject = CreateRenderObject();
        GetAttachedNode()->GetRuntimeWorld()->AddRenderObject(m_renderObject);
        m_renderObject->SetTransform(GetAttachedNode()->GetTransform()->GetLocalToWorldMatrix());
    }
    void Grid3DComponent::EndComponent()
    {
        base::EndComponent();
        GetAttachedNode()->GetRuntimeWorld()->RemoveRenderObject(m_renderObject);
        m_renderObject.reset();
    }

    sptr<rendering::RenderObject> Grid3DComponent::CreateRenderObject()
    {
        auto ro = new LineRenderObject();
        ro->m_vert = m_vert;
        ro->m_colors = m_colors;

        return mksptr(ro);
    }
    void Grid3DComponent::OnTick(Ticker ticker)
    {
        base::OnTick(ticker);
        m_renderObject->SetTransform(GetAttachedNode()->GetTransform()->GetLocalToWorldMatrix());
    }

} // namespace pulsared