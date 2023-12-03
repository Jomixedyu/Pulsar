#include "Components/StaticMeshRendererComponent.h"

#include "AssetManager.h"
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
        StaticMesh_ref m_staticMesh;
        array_list<Material_ref> m_materials;

        explicit StaticMeshRenderObject(StaticMesh_ref staticMesh, const array_list<Material_ref>& materials)
            : m_staticMesh(staticMesh), m_materials(materials)
        {
        }
        StaticMeshRenderObject() = default;
        StaticMeshRenderObject* SetStaticMesh(StaticMesh_ref mesh)
        {
            m_staticMesh = mesh;
            return this;
        }
        StaticMeshRenderObject* SetMaterials(const array_list<Material_ref>& materials)
        {
            m_materials = materials;
            return this;
        }
        void SubmitChange()
        {
            m_batchs.clear();

            for (auto& mat : m_materials)
            {
                auto& batch = m_batchs.emplace_back();
                batch.Topology = gfx::GFXPrimitiveTopology::TriangleList;
                batch.IsUsedIndices = true;
                batch.IsCastShadow = true;
                batch.IsUsedIndices = true;
                batch.IsReverseCulling = IsDetermiantNegative();
                batch.Material = mat;

                auto vertBuffers = m_staticMesh->GetGPUResourceVertexBuffers();
                auto indicesBuffers = m_staticMesh->GetGPUResourceIndicesBuffers();

                for (size_t i = 0; i < vertBuffers.size(); ++i)
                {
                    auto& element = batch.Elements.emplace_back();
                    element.Vertex = vertBuffers[i];
                    element.Indices = indicesBuffers[i];
                }
            }
        }
        void OnCreateResource() override
        {
            SubmitChange();
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
        auto ro = mksptr(new StaticMeshRenderObject());
        // m_staticMesh->CreateGPUResource();
        m_staticMesh->CreateGPUResource();
        for (auto mat : *m_materials)
        {
            mat->CreateGPUResource();
        }
        ro->SetStaticMesh(m_staticMesh)
            ->SetMaterials(*m_materials)
            ->SubmitChange();
        return ro;
    }

    void StaticMeshRendererComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        if (info->GetName() == NAMEOF(m_staticMesh))
        {
            OnMeshChanged();
        }
        else if (info->GetName() == NAMEOF(m_materials))
        {
            OnMaterialChanged();
        }
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
    void StaticMeshRendererComponent::BeginComponent()
    {
        base::BeginComponent();
        m_renderObject = sptr_static_cast<StaticMeshRenderObject>(CreateRenderObject());
        GetWorld()->AddRenderObject(m_renderObject);
    }
    void StaticMeshRendererComponent::EndComponent()
    {
        base::EndComponent();
        GetWorld()->RemoveRenderObject(m_renderObject);
        m_renderObject.reset();
    }
    void StaticMeshRendererComponent::OnMeshChanged()
    {
        if (m_renderObject)
        {
            m_renderObject->SetStaticMesh(m_staticMesh)->SubmitChange();
        }

        const auto matCount = m_staticMesh ? m_staticMesh->GetMaterialCount() : 0;
        m_materials->resize(matCount);

    }
    void StaticMeshRendererComponent::OnMaterialChanged()
    {
        if (m_renderObject)
        {
            for (auto& mat : *m_materials)
            {
                if (!mat.GetPtr())
                {
                    // load
                    GetAssetManager()->LoadAssetById(mat.handle);
                }
                mat->CreateGPUResource();
            }
            m_renderObject->SetMaterials(*m_materials)->SubmitChange();
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