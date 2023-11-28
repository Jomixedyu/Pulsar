#include "Components/StaticMeshRendererComponent.h"
#include "Components/MeshContainerComponent.h"
#include <Pulsar/Application.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Rendering/RenderContext.h>
#include <gfx/GFXBuffer.h>

namespace pulsar
{
    class StaticMeshRenderObject final : public rendering::RenderObject
    {
      public:
        array_list<rendering::MeshBatch> m_batchs;
        bool m_isCreatedResource = false;
        StaticMeshRendererComponent_ref m_staticMeshRenderer;

        StaticMeshRenderObject(StaticMeshRendererComponent_ref staticMeshRenderer)
            : m_staticMeshRenderer(staticMeshRenderer)
        {
        }

        void SetStaticMeshRenderer(StaticMeshRendererComponent_ref staticMeshRenderer)
        {
            if(!m_staticMesh)
            {
                m_active = false;
                return;
            }
            m_batchs.clear();

            rendering::MeshBatch batch;
            batch.IsUsedIndices = true;
            batch.IsCastShadow = true;

            batch.Material = staticMeshRenderer->GetMaterials();

            m_batchs.push_back()
            m_staticMesh->GetGPUResourceVertexBuffer();

        }
        void OnCreateResource() override
        {
            auto staticMesh = m_meshRednerer->GetStaticMesh();
            auto sectionCount = staticMesh->GetMeshSectionCount();
            for (size_t i = 0; i < sectionCount; i++)
            {
                auto& section = staticMesh->GetMeshSection(i);

                rendering::MeshBatch batch;
                auto vertex = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::Vertex, section.GetVertexAllocSize());
                auto indices = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::Index, section.GetIndicesAllocSize());

                rendering::MeshBatchElement mbElement;
                mbElement.Vertex = vertex;
                mbElement.Indices = indices;

                batch.Elements.push_back(mbElement);
                batch.Material = m_meshRednerer->GetMaterial(section.MaterialIndex);
                batch.IsUsedIndices = true;
                batch.Topology = gfx::GFXPrimitiveTopology::TriangleList;
                batch.IsReverseCulling = IsDetermiantNegative();

                m_batchs.push_back(std::move(batch));
            };
        }
        void OnDestroyResource() override
        {
            // delete m_meshBuffer;
        }

        virtual array_list<rendering::MeshBatch> GetMeshBatchs() override
        {
            return m_batchs;
        }
    };

    sptr<rendering::RenderObject> StaticMeshRendererComponent::CreateRenderObject()
    {
        m_renderObject = mksptr(new StaticMeshRenderObject(THIS_REF));
        return m_renderObject;
    }

    void StaticMeshRendererComponent::SetStaticMesh(StaticMesh_ref staticMesh)
    {
        m_staticMesh = staticMesh;
        OnMeshChanged();
    }
    Material_ref StaticMeshRendererComponent::GetMaterial(int index) const
    {
        return m_materials->at(index);
    }

    void StaticMeshRendererComponent::SetMaterial(int index, Material_ref material)
    {
        m_materials->at(index) = material;
    }
    void StaticMeshRendererComponent::OnMeshChanged()
    {
        if (!m_staticMesh)
        {
            m_renderObject.reset();
        }
    }

    // void StaticMeshRendererComponent::OnDraw()
    //{
    //     auto filter = this->get_node()->GetComponent<MeshContainerComponent>();
    //     if (!filter)
    //     {
    //         Logger::Log("empty mesh container", LogLevel::Warning);
    //         return;
    //     }

    //    auto mat = this->materials_->at(0);

    //    {
    //        //ShaderPassScope pass(mat->get_shader()->GetPass(0));
    //        //pass->SetUniformMatrix4fv("MODEL", Matrix4f::StaticScalar());
    //        //pass->SetUniformMatrix4fv("VIEW", RenderContext::GetCurrentCamera()->GetViewMat());
    //        //pass->SetUniformMatrix4fv("PROJECTION", RenderContext::GetCurrentCamera()->GetProjectionMat());

    //        //auto mesh = filter->get_mesh();
    //        //assert(IsValid(mesh));
    //        //assert(mesh->GetIsBindGPU());

    //        //auto a = mesh->GetRenderHandle();

    //        //glBindVertexArray(mesh->GetRenderHandle());

    //        //glDrawElements(GL_TRIANGLES, mesh->GetIndicesCount(), GL_UNSIGNED_INT, 0);
    //    }
    //}

} // namespace pulsar