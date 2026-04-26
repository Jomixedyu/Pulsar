#include "Components/StaticMeshRendererComponent.h"

#include "AssetManager.h"
#include "Components/RendererComponent.h"
#include <Pulsar/Application.h>
#include <Pulsar/Logger.h>
#include <gfx/GFXBuffer.h>

#include <Pulsar/Rendering/ShaderConfig.h>
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
        array_list<int32_t> m_priorities;

        gfx::GFXBuffer_sp m_meshConstantBuffer;
        gfx::GFXDescriptorSet_sp m_meshObjDescriptorSet;
        gfx::GFXDescriptorSetLayout_sp m_meshDescriptorSetLayout;

        StaticMeshRenderObject() = default;
        StaticMeshRenderObject* SetStaticMesh(RCPtr<StaticMesh> mesh)
        {
            m_staticMesh = std::move(mesh);
            return this;
        }
        StaticMeshRenderObject* SetMaterials(const array_list<RCPtr<Material>>& materials, const array_list<int32_t>& priorities)
        {
            m_materials = materials;
            m_priorities = priorities;
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

        std::string GetInterface() const override
        {
            return "RENDERER_STATICMESH";
        }
    };

    void StaticMeshRenderObject::SubmitChange()
    {
        m_batches.clear();

        if (!m_staticMesh)
            return;

        for (int matIndex = 0; matIndex < m_materials.size(); ++matIndex)
        {
            auto& slot = m_materials.at(matIndex);

            auto& batch = m_batches.emplace_back();
            batch.State.Topology = gfx::GFXPrimitiveTopology::TriangleList;
            batch.State.VertexLayouts = {StaticMesh::StaticGetVertexLayout()};
            batch.IsUsedIndices = true;
            batch.IsCastShadow = true;

            batch.Material  = slot;
            batch.Priority  = (matIndex < (int)m_priorities.size()) ? m_priorities.at(matIndex) : 0;

            // null / invalid material → fallback to Error material
            bool isInvalidMaterial = !batch.Material
                || !batch.Material->GetShader()
                || !batch.Material->CreateGPUResource();
            if (isInvalidMaterial)
            {
                batch.Material = AssetManager::Get()->LoadAsset<Material>("Engine/Materials/Error");
                if (batch.Material)
                    batch.Material->CreateGPUResource();
            }

            // still no valid material after fallback, skip
            if (!batch.Material || !batch.Material->GetShader())
            {
                m_batches.pop_back();
                continue;
            }

            // Fill Queue and CullMode from shader config Pass[0]
            if (batch.Material->GetShader() &&
                batch.Material->GetShader()->GetConfig() &&
                batch.Material->GetShader()->GetConfig()->Passes &&
                batch.Material->GetShader()->GetConfig()->Passes->size() > 0)
            {
                auto& pass0 = (*batch.Material->GetShader()->GetConfig()->Passes)[0];
                batch.Queue = pass0->Queue;
                if (pass0->GraphicsPipeline)
                {
                    batch.CullMode = pass0->GraphicsPipeline->CullMode;
                }
            }

            batch.Interface = GetInterface();
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
                perModelDesc.BufferSize   = sizeof(PerRendererData);
                perModelDesc.ElementSize  = sizeof(PerRendererData);

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
                    mat->CreateGPUResource();
            }
            ro->SetStaticMesh(m_staticMesh)
                ->SetMaterials(*m_materials, *m_priorities)
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
            if (mat)
            {
                out.push_back(mat.GetHandle());
            }
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
    StaticMeshRendererComponent::StaticMeshRendererComponent()
    {
        init_sptr_member(m_materials);
        init_sptr_member(m_priorities);
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

        auto transform = GetTransform();
        auto mat = transform->GetLocalToWorldMatrix();
        box.Min = mat * box.Min;
        box.Max = mat * box.Max;

        sphere.Radius = jmath::MaxComponent(jmath::Abs(transform->GetWorldScale())) * srcBounds.Radius;
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

        m_canDrawGizmo = true;
        GetWorld()->GetGizmosManager().AddGizmoComponent(self_ptr());

        OnTransformChanged();
    }
    void StaticMeshRendererComponent::EndComponent()
    {
        base::EndComponent();

        if (m_canDrawGizmo)
        {
            GetWorld()->GetGizmosManager().RemoveGizmoComponent(self_ptr());
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

    void StaticMeshRendererComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        base::OnDrawGizmo(painter, selected);

        if (!m_staticMesh) return;
        if (!m_showNormalGizmo && !m_showTangentGizmo && !m_showBitangentGizmo) return;

        const Matrix4f localToWorld = GetNode()->GetTransform()->GetLocalToWorldMatrix();
        const float worldScale = jmath::MaxComponent(jmath::Abs(GetNode()->GetTransform()->GetWorldScale()));

        for (int si = 0; si < (int)m_staticMesh->GetMeshSectionCount(); ++si)
        {
            auto& section = m_staticMesh->GetMeshSection(si);
            if (section.Positions.empty()) continue;

            bool hasNormal  = !section.Normals.empty();
            bool hasTangent = !section.Tangents.empty();

            const float baseLen = m_staticMesh->GetBounds().Radius * 0.05f * worldScale;

            // 稀疏采样：顶点太多时跳过一些
            const size_t vertCount = section.Positions.size();
            const int step = std::max(1, static_cast<int>(vertCount / 2000));

            for (size_t vi = 0; vi < vertCount; vi += step)
            {
                Vector3f localPos = section.Positions[vi];
                Vector4f worldPos4 = localToWorld * Vector4f(localPos.x, localPos.y, localPos.z, 1.0f);
                Vector3f worldPos{ worldPos4.x, worldPos4.y, worldPos4.z };

                if (m_showNormalGizmo && hasNormal)
                {
                    Vector3f localEnd = localPos + section.Normals[vi] * baseLen;
                    Vector4f worldEnd4 = localToWorld * Vector4f(localEnd.x, localEnd.y, localEnd.z, 1.0f);
                    Vector3f worldEnd{ worldEnd4.x, worldEnd4.y, worldEnd4.z };

                    StaticMeshVertex a{}, b{};
                    a.Position = worldPos; a.Color = { 0, 80, 255, 255 };
                    b.Position = worldEnd; b.Color = { 0, 80, 255, 255 };
                    painter->DrawLine(a, b);
                }

                if (m_showTangentGizmo && hasTangent)
                {
                    Vector3f t = section.Tangents[vi].xyz();
                    Vector3f localEnd = localPos + t * baseLen;
                    Vector4f worldEnd4 = localToWorld * Vector4f(localEnd.x, localEnd.y, localEnd.z, 1.0f);
                    Vector3f worldEnd{ worldEnd4.x, worldEnd4.y, worldEnd4.z };

                    StaticMeshVertex a{}, b{};
                    a.Position = worldPos; a.Color = { 255, 40, 40, 255 };
                    b.Position = worldEnd; b.Color = { 255, 40, 40, 255 };
                    painter->DrawLine(a, b);
                }

                if (m_showBitangentGizmo && hasNormal && hasTangent)
                {
                    Vector3f t = section.Tangents[vi].xyz();
                    float w = section.Tangents[vi].w;
                    Vector3f bitangent = Cross(section.Normals[vi], t) * w;
                    Vector3f localEnd = localPos + bitangent * baseLen;
                    Vector4f worldEnd4 = localToWorld * Vector4f(localEnd.x, localEnd.y, localEnd.z, 1.0f);
                    Vector3f worldEnd{ worldEnd4.x, worldEnd4.y, worldEnd4.z };

                    StaticMeshVertex a{}, b{};
                    a.Position = worldPos; a.Color = { 40, 220, 40, 255 };
                    b.Position = worldEnd; b.Color = { 40, 220, 40, 255 };
                    painter->DrawLine(a, b);
                }
            }
        }
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
            m_renderObject->SetMaterials(*m_materials, *m_priorities)->SubmitChange();
        }
        RebuildObserver();
    }

} // namespace pulsar