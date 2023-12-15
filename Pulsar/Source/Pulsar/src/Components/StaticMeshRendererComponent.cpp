#include "Components/StaticMeshRendererComponent.h"

#include "AssetManager.h"
#include "Components/MeshContainerComponent.h"
#include <Pulsar/Application.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Rendering/RenderContext.h>
#include <gfx/GFXBuffer.h>

namespace pulsar
{
    static gfx::GFXDescriptorSetLayout_wp MeshDescriptorSetLayout;

    class StaticMeshRenderObject final : public rendering::RenderObject
    {
    public:
        array_list<rendering::MeshBatch> m_batchs;
        StaticMesh_ref m_staticMesh;
        array_list<Material_ref> m_materials;

        gfx::GFXBuffer_sp m_meshConstantBuffer;
        gfx::GFXDescriptorSet_sp m_meshObjDescriptorSet;
        gfx::GFXDescriptorSetLayout_sp m_meshDescriptorSetLayout;

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
        void SubmitChange();
        void OnCreateResource() override;
        void OnDestroyResource() override
        {
            m_meshObjDescriptorSet.reset();
            m_meshDescriptorSetLayout.reset();
            m_meshConstantBuffer.reset();
        }

        void OnChangedTransform() override
        {
            m_meshConstantBuffer->Fill(&m_localToWorld);
        }

        array_list<rendering::MeshBatch> GetMeshBatchs() override
        {
            return m_batchs;
        }
    };
    void StaticMeshRenderObject::SubmitChange()
    {
        m_batchs.clear();

        if (!m_staticMesh)
            return;

        for (auto& mat : m_materials)
        {
            auto& batch = m_batchs.emplace_back();
            batch.State.Topology = gfx::GFXPrimitiveTopology::TriangleList;
            batch.State.IsReverseCulling = IsDetermiantNegative();
            batch.IsUsedIndices = true;
            batch.IsCastShadow = true;
            batch.IsUsedIndices = true;
            batch.Material = mat;
            if (batch.Material == nullptr || !batch.Material->CreateGPUResource())
            {
                batch.Material = GetAssetManager()->LoadAsset<Material>("Engine/Materials/Missing");
                batch.Material->CreateGPUResource();
            }

            batch.DescriptorSetLayout = m_meshDescriptorSetLayout;

            if (!m_staticMesh->IsCreatedGPUResource())
            {
                m_staticMesh->CreateGPUResource();
            }
            auto vertBuffers = m_staticMesh->GetGPUResourceVertexBuffers();
            auto indicesBuffers = m_staticMesh->GetGPUResourceIndicesBuffers();

            for (size_t i = 0; i < vertBuffers.size(); ++i)
            {
                auto& element = batch.Elements.emplace_back();
                element.Vertex = vertBuffers[i];
                element.Indices = indicesBuffers[i];
                element.ModelDescriptor = m_meshObjDescriptorSet;
            }
        }
    }
    void StaticMeshRenderObject::OnCreateResource()
    {
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


        SubmitChange();
    }



    sptr<rendering::RenderObject> StaticMeshRendererComponent::CreateRenderObject()
    {
        auto ro = mksptr(new StaticMeshRenderObject());
        // m_staticMesh->CreateGPUResource();
        if(m_staticMesh)
        {
            m_staticMesh->CreateGPUResource();
            for (auto mat : *m_materials)
            {
                if(mat)
                {
                    mat->CreateGPUResource();
                }
            }
            ro->SetStaticMesh(m_staticMesh)
                ->SetMaterials(*m_materials)
                ->SubmitChange();
        }
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
        if(index >= m_materials->size())
        {
            Logger::Log("valid index", LogLevel::Error);
            return;
        }

        m_materials->at(index) = material;
    }



    void StaticMeshRendererComponent::BeginComponent()
    {
        base::BeginComponent();
        m_renderObject = sptr_static_cast<StaticMeshRenderObject>(CreateRenderObject());
        GetWorld()->AddRenderObject(m_renderObject);
        OnMsg_TransformChanged();
    }
    void StaticMeshRendererComponent::EndComponent()
    {
        base::EndComponent();
        GetWorld()->RemoveRenderObject(m_renderObject);
        m_renderObject.reset();

    }
    void StaticMeshRendererComponent::OnReceiveMessage(MessageId id)
    {
        base::OnReceiveMessage(id);
    }
    void StaticMeshRendererComponent::OnMsg_TransformChanged()
    {
        base::OnMsg_TransformChanged();
        m_renderObject->SetTransform(GetAttachedNode()->GetTransform()->GetLocalToWorldMatrix());
    }
    void StaticMeshRendererComponent::OnMeshChanged()
    {
        if (m_staticMesh)
        {
            TryFindOrLoadObject(m_staticMesh);
            if(m_staticMesh->GetMaterialCount() > m_materials->size())
            {
                m_materials->resize(m_staticMesh->GetMaterialCount());
            }
        }

        if (m_renderObject)
        {
            m_renderObject->SetStaticMesh(m_staticMesh)->SubmitChange();
        }

    }
    void StaticMeshRendererComponent::OnMaterialChanged()
    {
        if (m_renderObject)
        {
            for (auto& mat : *m_materials)
            {
                if (!mat.GetPtr())
                {
                    TryFindOrLoadObject(mat);
                }
            }
            m_renderObject->SetMaterials(*m_materials)->SubmitChange();
        }
    }

} // namespace pulsar