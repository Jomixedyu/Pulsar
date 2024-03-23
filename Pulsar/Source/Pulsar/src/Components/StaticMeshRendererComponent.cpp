#include "Components/StaticMeshRendererComponent.h"

#include "AssetManager.h"
#include "Components/MeshContainerComponent.h"
#include <Pulsar/Application.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Rendering/RenderContext.h>
#include <gfx/GFXBuffer.h>

#include <utility>

namespace pulsar
{
    static gfx::GFXDescriptorSetLayout_wp MeshDescriptorSetLayout;

    class StaticMeshRenderObject final : public rendering::RenderObject
    {
    public:
        array_list<rendering::MeshBatch> m_batchs;
        RCPtr<StaticMesh> m_staticMesh;
        array_list<RCPtr<Material>> m_materials;

        gfx::GFXBuffer_sp m_meshConstantBuffer;
        gfx::GFXDescriptorSet_sp m_meshObjDescriptorSet;
        gfx::GFXDescriptorSetLayout_sp m_meshDescriptorSetLayout;

        explicit StaticMeshRenderObject(RCPtr<StaticMesh> staticMesh, const array_list<RCPtr<Material>>& materials)
            : m_staticMesh(std::move(staticMesh)), m_materials(materials)
        {
        }
        StaticMeshRenderObject() = default;
        StaticMeshRenderObject* SetStaticMesh(RCPtr<StaticMesh> mesh)
        {
            m_staticMesh = std::move(mesh);
            return this;
        }
        StaticMeshRenderObject* SetMaterials(const array_list<RCPtr<Material>>& materials)
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
            for (auto& batch : m_batchs)
            {
                batch.IsReverseCulling = IsDetermiantNegative();
            }
            m_meshConstantBuffer->Fill(&m_perModelData);
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

            batch.State.VertexLayouts = {StaticMesh::StaticGetVertexLayout()};
            batch.IsUsedIndices = true;
            batch.IsCastShadow = true;
            batch.IsUsedIndices = true;
            batch.Material = mat;
            if (batch.Material == nullptr || !batch.Material->CreateGPUResource())
            {
                batch.Material = GetAssetManager()->LoadAsset<Material>("Engine/Materials/Missing");
                batch.Material->CreateGPUResource();
            }
            batch.CullMode = batch.Material->GetShader()->GetConfig()->CullMode;

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

        SubmitChange();
    }





    SPtr<rendering::RenderObject> StaticMeshRendererComponent::CreateRenderObject()
    {
        auto ro = mksptr(new StaticMeshRenderObject());
        // m_staticMesh->CreateGPUResource();
        if (m_staticMesh)
        {
            m_staticMesh->CreateGPUResource();
            for (const auto& mat : *m_materials)
            {
                if (mat)
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
            SetStaticMesh(m_staticMesh);
        }
        else if (info->GetName() == NAMEOF(m_materials))
        {
            if (m_materialsSize != m_materials->size())
            {
                ResizeMaterials(m_materials->size());
            }
            OnMaterialChanged();
        }
    }
    StaticMeshRendererComponent::StaticMeshRendererComponent() :
        CORELIB_INIT_INTERFACE(IRendererComponent)
    {
        init_sptr_member(m_materials);
    }

    Bounds3f StaticMeshRendererComponent::GetBounds()
    {
        auto box = m_staticMesh->GetBounds().GetBox();
        auto mat = GetTransform()->GetLocalToWorldMatrix();
        box.Min = mat * box.Min;
        box.Max = mat * box.Max;

        return Bounds3f{ box };
    }

    void StaticMeshRendererComponent::SetStaticMesh(RCPtr<StaticMesh> staticMesh)
    {
        if (m_staticMesh)
        {
            RuntimeObjectManager::RemoveDependList(GetObjectHandle(), m_staticMesh.GetHandle());
        }
        m_staticMesh = std::move(staticMesh);
        if (m_staticMesh)
        {
            RuntimeObjectManager::AddDependList(GetObjectHandle(), m_staticMesh.GetHandle());
        }

        OnMeshChanged();
    }
    RCPtr<StaticMesh> StaticMeshRendererComponent::GetMaterial(int index) const
    {
        return m_materials->at(index);
    }

    void StaticMeshRendererComponent::SetMaterial(int index, RCPtr<Material> material)
    {
        if (index >= m_materials->size())
        {
            Logger::Log("valid index", LogLevel::Error);
            return;
        }


        if (m_beginning)
        {
            if (auto& mat = m_materials->at(index))
            {
                RuntimeObjectManager::RemoveDependList(GetObjectHandle(), mat.GetHandle());
            }
        }

        m_materials->at(index) = material;

        if (m_beginning)
        {
            if (material)
            {
                RuntimeObjectManager::AddDependList(GetObjectHandle(), material.GetHandle());
            }
        }


        OnMaterialChanged();
    }
    size_t StaticMeshRendererComponent::AddMaterial()
    {
        ResizeMaterials(m_materials->size() + 1);
        return m_materialsSize - 1;
    }
    void StaticMeshRendererComponent::RemoveMaterial(size_t index)
    {
        // todo
    }

    void StaticMeshRendererComponent::BeginComponent()
    {
        base::BeginComponent();
        m_renderObject = sptr_static_cast<StaticMeshRenderObject>(CreateRenderObject());
        GetWorld()->AddRenderObject(m_renderObject);
        ResizeMaterials(m_materials->size());

        for (size_t i = 0; i < m_materials->GetCount(); ++i)
        {
            if (auto& mat = m_materials->at(i))
            {
                RuntimeObjectManager::AddDependList(GetObjectHandle(), mat.GetHandle());
            }
        }
        OnMsg_TransformChanged();
    }
    void StaticMeshRendererComponent::EndComponent()
    {
        base::EndComponent();

        for (size_t i = 0; i < m_materials->GetCount(); ++i)
        {
            if (auto& mat = m_materials->at(i))
            {
                RuntimeObjectManager::RemoveDependList(GetObjectHandle(), mat.GetHandle());
            }
        }

        GetWorld()->RemoveRenderObject(m_renderObject);
        m_renderObject.reset();
    }
    // void StaticMeshRendererComponent::BeginListenMaterialStateChanged(size_t index)
    // {
    //     if (auto mat = m_materials->at(index))
    //     {
    //         auto handle = mat->OnShaderChanged.AddListener(this, &ThisClass::OnMaterialStateChanged);
    //         m_materialStateChangedCallbacks[index] = handle;
    //     }
    //     else
    //     {
    //         m_materialStateChangedCallbacks[index] = 0;
    //     }
    // }
    // void StaticMeshRendererComponent::EndListenMaterialStateChanged(size_t index)
    // {
    //     auto handle = m_materialStateChangedCallbacks[index];
    //     m_materialStateChangedCallbacks[index] = 0;
    //     if (auto mat = m_materials->at(index))
    //     {
    //         mat->OnShaderChanged.RemoveListenerByIndex(handle);
    //     }
    // }
    void StaticMeshRendererComponent::OnMaterialStateChanged()
    {
        this->OnMaterialChanged();
    }

    void StaticMeshRendererComponent::OnReceiveMessage(MessageId id)
    {
        base::OnReceiveMessage(id);
    }

    void StaticMeshRendererComponent::OnDependencyMessage(ObjectHandle inDependency, DependencyObjectState msg)
    {
        base::OnDependencyMessage(inDependency, msg);
        if (EnumHasFlag(msg, DependencyObjectState::Reload))
        {
            ObjectPtr<ObjectBase> obj = inDependency;
            if (!obj) return;
            if (obj->GetType() == cltypeof<Material>())
            {

            }
            else if(obj->GetType() == cltypeof<StaticMesh>())
            {

            }
        }
        else if(EnumHasFlag(msg, DependencyObjectState::Unload))
        {

        }

    }
    void StaticMeshRendererComponent::ResizeMaterials(size_t size)
    {
        m_materialsSize = size;

        if (size < m_materials->size())
        {
            for (size_t i = size; i < m_materialsSize; ++i)
            {
                if (auto& mat = m_materials->at(i))
                {
                    RuntimeObjectManager::RemoveDependList(GetObjectHandle(), mat.GetHandle());
                }
            }
        }

        m_materials->resize(size);
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
            TryLoadAssetRCPtr(m_staticMesh);
            if (m_staticMesh->GetMaterialCount() > m_materials->size())
            {
                ResizeMaterials(m_staticMesh->GetMaterialCount());
                OnMaterialChanged();
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
                    TryLoadAssetRCPtr(mat);
                }
            }
            m_renderObject->SetMaterials(*m_materials)->SubmitChange();
        }
    }

} // namespace pulsar