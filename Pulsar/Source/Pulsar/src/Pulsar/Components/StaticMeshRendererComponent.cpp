#include "Components/StaticMeshRendererComponent.h"

#include "AssetManager.h"
#include "Components/MeshRendererComponent.h"
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
        array_list<rendering::MeshBatch> m_batches;
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
            for (auto& batch : m_batches)
            {
                batch.IsReverseCulling = IsDeterminantNegative();
            }
            m_meshConstantBuffer->Fill(&m_perModelData);
        }

        array_list<rendering::MeshBatch> GetMeshBatches() override
        {
            return m_batches;
        }
    };

    void StaticMeshRenderObject::SubmitChange()
    {
        m_batches.clear();

        if (!m_staticMesh)
            return;

        for (int matIndex = 0; matIndex < m_materials.size(); ++matIndex)
        {
            auto& mat = m_materials.at(matIndex);
            if (!mat || !mat->GetShader())
            {
                continue;
            }
            auto& batch = m_batches.emplace_back();
            batch.State.Topology = gfx::GFXPrimitiveTopology::TriangleList;

            batch.State.VertexLayouts = {StaticMesh::StaticGetVertexLayout()};
            batch.IsUsedIndices = true;
            batch.IsCastShadow = true;
            batch.IsUsedIndices = true;
            batch.Material = mat;
            bool isInvalidMaterial = false;
            isInvalidMaterial = batch.Material == nullptr || !batch.Material->CreateGPUResource();
            isInvalidMaterial = isInvalidMaterial || (batch.Material && batch.Material->GetShader()->GetConfig()->RenderingType == ShaderPassRenderingType::PostProcessing);
            if (isInvalidMaterial)
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

            // collect elements
            auto vertBuffers = m_staticMesh->GetGPUResourceVertexBuffers();
            auto indicesBuffers = m_staticMesh->GetGPUResourceIndicesBuffers();

            if (matIndex < vertBuffers.size())
            {
                auto& element = batch.Elements.emplace_back();
                element.Vertex =  vertBuffers[matIndex];
                element.Indices = indicesBuffers[matIndex];
                element.ModelDescriptor = m_meshObjDescriptorSet;
            }

        }

        if (m_batches.empty())
        {
            // empty
        }
    }

    void StaticMeshRenderObject::OnCreateResource()
    {
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

        gfx::GFXBufferDesc perModelDesc{};
        perModelDesc.Usage        = gfx::GFXBufferUsage::ConstantBuffer;
        perModelDesc.StorageType  = gfx::GFXBufferMemoryPosition::VisibleOnDevice;
        perModelDesc.BufferSize   = sizeof(PerModelShaderParameter);
        perModelDesc.ElementSize  = sizeof(PerModelShaderParameter);

        m_meshConstantBuffer   = Application::GetGfxApp()->CreateBuffer(perModelDesc);
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
    void StaticMeshRendererComponent::GetSubscribeObserverHandles(array_list<ObjectHandle>& out)
    {
        base::GetSubscribeObserverHandles(out);
        out.push_back(m_staticMesh.GetHandle());
        for (auto& mat : *m_materials)
        {
            out.push_back(mat.GetHandle());
        }
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

    BoxSphereBounds3f StaticMeshRendererComponent::GetBoundsWS()
    {
        if (!m_staticMesh)
        {
            return {};
        }
        auto srcBounds = m_staticMesh->GetBounds();
        auto box = srcBounds.GetBox();
        auto sphere = srcBounds.GetSphere();

        auto mat = GetTransform()->GetLocalToWorldMatrix();
        box.Min = mat * box.Min;
        box.Max = mat * box.Max;

        sphere.Radius = jmath::MaxComponent(jmath::Abs(GetTransform()->GetWorldScale())) * srcBounds.Radius;
        return BoxSphereBounds3f{box, sphere};
    }

    void StaticMeshRendererComponent::SetStaticMesh(RCPtr<StaticMesh> staticMesh)
    {
        m_staticMesh = std::move(staticMesh);

        OnMeshChanged();
    }
    RCPtr<Material> StaticMeshRendererComponent::GetMaterial(int index) const
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

        m_materials->at(index) = std::move(material);

        OnMaterialChanged();
    }
    size_t StaticMeshRendererComponent::AddMaterial(RCPtr<Material> material)
    {
        ResizeMaterials(m_materials->size() + 1);
        if (material)
        {
            SetMaterial((int)m_materialsSize - 1, material);
        }
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

        OnTransformChanged();
    }
    void StaticMeshRendererComponent::EndComponent()
    {
        base::EndComponent();

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

    void StaticMeshRendererComponent::OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg)
    {
        base::OnNotifyObserver(inDependency, msg);
        if (EnumHasFlag(msg, DependencyObjectState::Modified))
        {
            ObjectPtrBase obj = inDependency;
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
        m_materials->resize(size);
        RebuildObserver();
    }

    void StaticMeshRendererComponent::OnTransformChanged()
    {
        base::OnTransformChanged();
        m_renderObject->SetTransform(GetNode()->GetTransform()->GetLocalToWorldMatrix());
    }

    void StaticMeshRendererComponent::OnMeshChanged()
    {
        if (m_staticMesh)
        {
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
        RebuildObserver();
    }
    void StaticMeshRendererComponent::OnMaterialChanged()
    {
        if (m_renderObject)
        {
            m_renderObject->SetMaterials(*m_materials)->SubmitChange();
        }
        RebuildObserver();
    }

} // namespace pulsar