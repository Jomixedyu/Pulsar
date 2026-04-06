#include "Components/SkinnedMeshRendererComponent.h"

#include "AssetManager.h"
#include <Pulsar/Application.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Rendering/ShaderConfig.h>
#include <gfx/GFXBuffer.h>

namespace pulsar
{
    // -----------------------------------------------------------------------
    // SkinnedMeshRenderObject
    // -----------------------------------------------------------------------
    static gfx::GFXDescriptorSetLayout_wp SkinnedMeshDescriptorSetLayout;

    class SkinnedMeshRenderObject final : public rendering::RenderObject
    {
    public:
        array_list<rendering::MeshBatch> m_batches;
        RCPtr<SkinnedMesh>               m_skinnedMesh;
        array_list<SPtr<MaterialSlot>>   m_materials;

        // set2 binding0: PerRendererData
        gfx::GFXBuffer_sp             m_perRendererBuffer;
        // set2 binding1: SkinnedRendererData (BoneMatrices)
        gfx::GFXBuffer_sp             m_skinningBuffer;

        gfx::GFXDescriptorSet_sp      m_descriptorSet;
        gfx::GFXDescriptorSetLayout_sp m_descriptorSetLayout;

        SkinnedMeshRenderObject* SetSkinnedMesh(RCPtr<SkinnedMesh> mesh)
        {
            m_skinnedMesh = std::move(mesh);
            return this;
        }
        SkinnedMeshRenderObject* SetMaterials(const array_list<SPtr<MaterialSlot>>& mats)
        {
            m_materials = mats;
            return this;
        }

        // Animator 调用：将骨骼矩阵写入 GPU UBO
        void UploadBoneMatrices(const array_list<Matrix4f>& boneMatrices)
        {
            if (!m_skinningBuffer) return;

            SkinnedRendererData data{};
            const size_t count = std::min(boneMatrices.size(), (size_t)SKINNEDMESH_MAX_BONES);
            for (size_t i = 0; i < count; ++i)
                data.BoneMatrices[i] = boneMatrices[i];

            m_skinningBuffer->Fill(&data);
        }

        void SubmitChange();
        void OnCreateResource() override;
        void OnDestroyResource() override
        {
            m_descriptorSet.reset();
            m_descriptorSetLayout.reset();
            m_perRendererBuffer.reset();
            m_skinningBuffer.reset();
        }

        void OnChangedTransform() override
        {
            for (auto& batch : m_batches)
                batch.IsReverseCulling = IsDeterminantNegative();
            m_perRendererBuffer->Fill(&m_perModelData);
        }

        array_list<rendering::MeshBatch> GetMeshBatches() override { return m_batches; }
        std::string GetInterface() const override { return "RENDERER_SKINNEDMESH"; }
    };

    void SkinnedMeshRenderObject::OnCreateResource()
    {
        // set2 layout: binding0=PerRendererData, binding1=SkinnedRendererData
        if (SkinnedMeshDescriptorSetLayout.expired())
        {
            gfx::GFXDescriptorSetLayoutDesc bindings[2] = {
                {gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXGpuProgramStageFlags::VertexFragment,
                 kRenderingDescriptorBinding_PerRenderer,  kRenderingDescriptorSpace_ModelInfo},
                {gfx::GFXDescriptorType::ConstantBuffer, gfx::GFXGpuProgramStageFlags::VertexFragment,
                 kRenderingDescriptorBinding_SkinningData, kRenderingDescriptorSpace_ModelInfo},
            };
            m_descriptorSetLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(bindings, 2);
            SkinnedMeshDescriptorSetLayout = m_descriptorSetLayout;
        }
        else
        {
            m_descriptorSetLayout = SkinnedMeshDescriptorSetLayout.lock();
        }

        // binding0: PerRendererData
        {
            gfx::GFXBufferDesc desc{};
            desc.Usage       = gfx::GFXBufferUsage::ConstantBuffer;
            desc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnDevice;
            desc.BufferSize  = sizeof(PerRendererData);
            desc.ElementSize = sizeof(PerRendererData);
            m_perRendererBuffer = Application::GetGfxApp()->CreateBuffer(desc);
        }

        // binding1: SkinnedRendererData（初始化为单位矩阵）
        {
            gfx::GFXBufferDesc desc{};
            desc.Usage       = gfx::GFXBufferUsage::ConstantBuffer;
            desc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnDevice;
            desc.BufferSize  = sizeof(SkinnedRendererData);
            desc.ElementSize = sizeof(SkinnedRendererData);
            m_skinningBuffer = Application::GetGfxApp()->CreateBuffer(desc);

            // 默认骨骼矩阵全部为单位矩阵（静止姿势）
            SkinnedRendererData defaultData{};
            for (auto& mat : defaultData.BoneMatrices)
                mat = Matrix4f::Identity();
            m_skinningBuffer->Fill(&defaultData);
        }

        m_descriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_descriptorSetLayout);
        m_descriptorSet->AddDescriptor("PerRenderer", kRenderingDescriptorBinding_PerRenderer)
                       ->SetConstantBuffer(m_perRendererBuffer.get());
        m_descriptorSet->AddDescriptor("SkinningData", kRenderingDescriptorBinding_SkinningData)
                       ->SetConstantBuffer(m_skinningBuffer.get());
        m_descriptorSet->Submit();

        SubmitChange();
    }

    void SkinnedMeshRenderObject::SubmitChange()
    {
        m_batches.clear();
        if (!m_skinnedMesh) return;

        for (int matIndex = 0; matIndex < (int)m_materials.size(); ++matIndex)
        {
            auto& slot = m_materials.at(matIndex);

            auto& batch = m_batches.emplace_back();
            batch.State.Topology  = gfx::GFXPrimitiveTopology::TriangleList;
            batch.State.VertexLayouts = {SkinnedMesh::StaticGetVertexLayout()};
            batch.IsUsedIndices   = true;
            batch.IsCastShadow    = true;
            batch.Material        = (slot ? slot->material : nullptr);
            batch.Priority        = (slot ? slot->priority : 0);

            bool isInvalidMaterial = !batch.Material
                || !batch.Material->GetShader()
                || !batch.Material->CreateGPUResource();
            if (isInvalidMaterial)
            {
                batch.Material = AssetManager::Get()->LoadAsset<Material>("Engine/Materials/Error");
                if (batch.Material) batch.Material->CreateGPUResource();
            }
            if (!batch.Material || !batch.Material->GetShader())
            {
                m_batches.pop_back();
                continue;
            }

            if (batch.Material->GetShader()->GetConfig() &&
                batch.Material->GetShader()->GetConfig()->Passes &&
                !batch.Material->GetShader()->GetConfig()->Passes->empty())
            {
                auto& pass0 = (*batch.Material->GetShader()->GetConfig()->Passes)[0];
                batch.Queue = pass0->Queue;
                if (pass0->GraphicsPipeline)
                    batch.CullMode = pass0->GraphicsPipeline->CullMode;
            }

            batch.Interface           = GetInterface();
            batch.DescriptorSetLayout = m_descriptorSetLayout;

            if (!m_skinnedMesh->IsCreatedGPUResource())
                m_skinnedMesh->CreateGPUResource();

            auto vertBuffers    = m_skinnedMesh->GetGPUResourceVertexBuffers();
            auto indicesBuffers = m_skinnedMesh->GetGPUResourceIndicesBuffers();

            if (matIndex < (int)vertBuffers.size())
            {
                auto& element         = batch.Elements.emplace_back();
                element.Vertex        = vertBuffers[matIndex];
                element.Indices       = indicesBuffers[matIndex];
                element.ModelDescriptor = m_descriptorSet;
            }
        }
    }

    // -----------------------------------------------------------------------
    // SkinnedMeshRendererComponent
    // -----------------------------------------------------------------------
    SkinnedMeshRendererComponent::SkinnedMeshRendererComponent()
    {
        m_canDrawGizmo = true;
        init_sptr_member(m_materials);
    }

    SPtr<rendering::RenderObject> SkinnedMeshRendererComponent::CreateRenderObject()
    {
        auto ro = mksptr(new SkinnedMeshRenderObject());
        if (m_skinnedMesh)
        {
            m_skinnedMesh->CreateGPUResource();
            for (const auto& mat : *m_materials)
            {
                if (mat && mat->material)
                    mat->material->CreateGPUResource();
            }
            ro->SetSkinnedMesh(m_skinnedMesh)
              ->SetMaterials(*m_materials)
              ->SubmitChange();
        }
        return ro;
    }

    void SkinnedMeshRendererComponent::BeginComponent()
    {
        base::BeginComponent();
        m_renderObject = sptr_static_cast<SkinnedMeshRenderObject>(CreateRenderObject());
        GetWorld()->AddRenderObject(m_renderObject);
        ResizeMaterials(m_materials->size());
        OnTransformChanged();
    }

    void SkinnedMeshRendererComponent::EndComponent()
    {
        base::EndComponent();
        GetWorld()->RemoveRenderObject(m_renderObject);
        m_renderObject.reset();
    }

    void SkinnedMeshRendererComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        if (info->GetName() == NAMEOF(m_skinnedMesh))
        {
            SetSkinnedMesh(m_skinnedMesh);
        }
        else if (info->GetName() == NAMEOF(m_materials))
        {
            if (m_materialsSize != m_materials->size())
                ResizeMaterials(m_materials->size());
            OnMaterialChanged();
        }
    }

    void SkinnedMeshRendererComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        base::OnDrawGizmo(painter, selected);
    }

    void SkinnedMeshRendererComponent::SetSkinnedMesh(const RCPtr<SkinnedMesh>& mesh)
    {
        m_skinnedMesh = mesh;
        OnMeshChanged();
    }

    void SkinnedMeshRendererComponent::UpdateBoneMatrices(const array_list<Matrix4f>& boneMatrices)
    {
        if (m_renderObject)
            m_renderObject->UploadBoneMatrices(boneMatrices);
    }

    RCPtr<Material> SkinnedMeshRendererComponent::GetMaterial(int index) const
    {
        return m_materials->at(index)->material;
    }

    void SkinnedMeshRendererComponent::SetMaterial(int index, RCPtr<Material> material)
    {
        if (index >= (int)m_materials->size()) return;
        m_materials->at(index)->material = std::move(material);
        OnMaterialChanged();
    }

    size_t SkinnedMeshRendererComponent::AddMaterial(RCPtr<Material> material)
    {
        ResizeMaterials(m_materials->size() + 1);
        if (material) SetMaterial((int)m_materialsSize - 1, material);
        return m_materialsSize - 1;
    }

    void SkinnedMeshRendererComponent::GetSubscribeObserverHandles(array_list<ObjectHandle>& out)
    {
        base::GetSubscribeObserverHandles(out);
        out.push_back(m_skinnedMesh.GetHandle());
        for (auto& mat : *m_materials)
        {
            if (mat) out.push_back(mat->material.GetHandle());
        }
    }

    void SkinnedMeshRendererComponent::OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg)
    {
        base::OnNotifyObserver(inDependency, msg);
    }

    void SkinnedMeshRendererComponent::ResizeMaterials(size_t size)
    {
        size_t oldSize = m_materials->size();
        m_materialsSize = size;
        m_materials->resize(size);
        for (size_t i = oldSize; i < size; ++i)
        {
            if (!m_materials->at(i))
                m_materials->at(i) = mksptr(new MaterialSlot());
        }
        RebuildObserver();
    }

    void SkinnedMeshRendererComponent::OnTransformChanged()
    {
        base::OnTransformChanged();
        if (m_renderObject)
            m_renderObject->SetTransform(GetNode()->GetTransform()->GetLocalToWorldMatrix());
    }

    void SkinnedMeshRendererComponent::OnMeshChanged()
    {
        if (m_skinnedMesh && m_skinnedMesh->GetMaterialCount() > m_materials->size())
        {
            ResizeMaterials(m_skinnedMesh->GetMaterialCount());
            OnMaterialChanged();
        }
        if (m_renderObject)
            m_renderObject->SetSkinnedMesh(m_skinnedMesh)->SubmitChange();
        RebuildObserver();
    }

    void SkinnedMeshRendererComponent::OnMaterialChanged()
    {
        if (m_renderObject)
            m_renderObject->SetMaterials(*m_materials)->SubmitChange();
        RebuildObserver();
    }

} // namespace pulsar
