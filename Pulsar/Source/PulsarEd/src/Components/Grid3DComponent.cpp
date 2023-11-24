#include "Components/Grid3DComponent.h"
#include <Pulsar/Rendering/ShaderPass.h>
#include <PulsarEd/Windows/EditorWindowManager.h>
#include <PulsarEd/Windows/SceneWindow.h>
#include <Pulsar/Assets/StaticMesh.h>
#include <gfx/GFXShaderPass.h>
#include <Pulsar/AssetManager.h>

namespace pulsared
{

    void Grid3DComponent::BeginComponent()
    {
        base::BeginComponent();

        int line_count = 20;
        float detail_distance = 1;
        float total_width = detail_distance * line_count;

        for (int x = -line_count / 2; x <= line_count / 2; x++)
        {
            m_vert.push_back({ total_width / 2, 0, detail_distance * x });
            m_vert.push_back({ -total_width / 2, 0 , detail_distance * x });

            Color4f color = { 0.1f, 0.1f, 0.1f, 1 };
            if (x == 0)
            {
                color.r = 0.9f;
            }
            m_colors.push_back(jmath::FloatColorToBitColor<uint8_t>(color));
            m_colors.push_back(jmath::FloatColorToBitColor<uint8_t>(color));
        }
        for (int z = -line_count / 2; z <= line_count / 2; z++)
        {
            m_vert.push_back({ detail_distance * z, 0, total_width / 2 });
            m_vert.push_back({ detail_distance * z, 0, -total_width / 2 });
            Color4f color = { 0.1f, 0.1f, 0.1f, 1 };
            if (z == 0)
            {
                color.r = 0.2f;
                color.g = 0.2f;
                color.b = 1.f;
            }
            m_colors.push_back(jmath::FloatColorToBitColor<uint8_t>(color));
            m_colors.push_back(jmath::FloatColorToBitColor<uint8_t>(color));
        }

        m_renderObject = CreateRenderObject();
        //GetAttachedNode()->GetRuntimeWorld()->AddRenderObject(m_renderObject);
    }
    void Grid3DComponent::EndComponent()
    {
        base::EndComponent();
        //GetAttachedNode()->GetRuntimeWorld()->RemoveRenderObject(m_renderObject);
        m_renderObject.reset();
    }

    class LineRenderObject : public rendering::RenderObject
    {
        using base = rendering::RenderObject;
    public:
        array_list<Vector3f> m_vert;
        array_list<Color4b> m_colors;
        array_list<StaticMeshVertex> m_verties;

        gfx::GFXBuffer_sp m_vertBuffer;

        array_list<rendering::MeshBatch> m_batchs;
    public:
        LineRenderObject()
        {
        }

        virtual void OnCreateResource() override
        {
            base::OnCreateResource();
            for (size_t i = 0; i < m_vert.size(); i++)
            {
                StaticMeshVertex vert;
                vert.Color = m_colors[i];
                vert.Position = m_vert[i];

                m_verties.push_back(vert);
            }

            m_vertBuffer = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::Vertex, m_verties.size() * sizeof(StaticMeshVertex));
            m_vertBuffer->Fill(m_verties.data());

            m_batchs.resize(1);
            rendering::MeshBatch& batch = m_batchs[0];

            batch.Elements.resize(1);
            batch.Elements[0].Vertex = m_vertBuffer;
            batch.Topology = gfx::GFXPrimitiveTopology::LineList;
            batch.IsUsedIndices = false;
            batch.Material = GetAssetManager()->LoadAsset<Material>("Engine/Materials/Line");
        }
        virtual void OnDestroyResource() override
        {
            base::OnDestroyResource();
            m_vertBuffer.reset();
        }
        virtual array_list<rendering::MeshBatch> GetMeshBatchs()
        {
            for (auto batch : m_batchs)
            {
                if (!batch.Material->IsCreatedGPUResource())
                {
                    batch.Material->CreateGPUResource();
                }
            }
            return m_batchs;
        }
    };

    sptr<rendering::RenderObject> Grid3DComponent::CreateRenderObject()
    {
        auto ro = new LineRenderObject();
        ro->m_vert = m_vert;
        ro->m_colors = m_colors;
        
        return mksptr(ro);
    }

}