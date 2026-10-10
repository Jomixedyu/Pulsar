#include "Components/StaticMeshRendererComponent.h"

#include "AssetManager.h"
#include <Pulsar/Application.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Rendering/MaterialProxy.h>
#include <Pulsar/Rendering/MeshProxy.h>
#include <Pulsar/Rendering/RenderThread.h>
#include <gfx/GFXBuffer.h>

#include <Pulsar/Rendering/ShaderConfig.h>
#include <utility>

namespace pulsar
{
    namespace
    {
        // Game thread: resolve material assets to render proxies, falling back
        // to the Error material for null / invalid entries.
        array_list<std::shared_ptr<MaterialProxy>> ResolveMaterialProxies(const array_list<RCPtr<Material>>& materials)
        {
            array_list<std::shared_ptr<MaterialProxy>> result;
            result.reserve(materials.size());
            for (auto mat : materials)
            {
                if (!mat || !mat->GetShader() || !mat->EnsureGPUResource())
                    mat = AssetManager::Get()->LoadAsset<Material>("Pulsar/Materials/Error");
                if (!mat || !mat->GetShader() || !mat->EnsureGPUResource())
                {
                    result.push_back(nullptr); // no valid material even after fallback
                    continue;
                }
                result.push_back(mat->GetRenderProxy());
            }
            return result;
        }
    }

    class StaticMeshRenderObject final : public rendering::RenderObjectProxy
    {
    public:
        array_list<rendering::MeshBatch> m_batches;
        std::shared_ptr<rendering::MeshProxy> m_meshProxy;
        array_list<std::shared_ptr<MaterialProxy>> m_materialProxies;
        array_list<int32_t> m_priorities;

        gfx::GFXDescriptorSetPtr m_dummyExtraSet;

        StaticMeshRenderObject() = default;
        StaticMeshRenderObject* SetMeshProxy(std::shared_ptr<rendering::MeshProxy> mesh)
        {
            m_meshProxy = std::move(mesh);
            return this;
        }
        StaticMeshRenderObject* SetMaterialProxies(const array_list<std::shared_ptr<MaterialProxy>>& materials, const array_list<int32_t>& priorities)
        {
            m_materialProxies = materials;
            m_priorities = priorities;
            return this;
        }
        void SubmitChange();
        void OnEnterScene() override;
        void OnLeaveScene() override
        {
            m_dummyExtraSet.reset();
        }

        void OnChangedTransform() override
        {
            for (auto& batch : m_batches)
            {
                batch.IsReverseCulling = IsDeterminantNegative();
            }
        }

        array_list<rendering::MeshBatch> GetMeshBatches() override
        {
            return m_batches;
        }

    };

    void StaticMeshRenderObject::SubmitChange()
    {
        m_batches.clear();

        if (!m_meshProxy)
            return;

        // This object only ever sees proxies; GPU resources were ensured by the
        // component on the game thread.
        const auto& vertBuffers = m_meshProxy->GetVertexBuffers();
        const auto& indexBuffers = m_meshProxy->GetIndexBuffers();

        for (int matIndex = 0; matIndex < m_materialProxies.size(); ++matIndex)
        {
            auto& batch = m_batches.emplace_back();
            batch.State.Topology = gfx::GFXPrimitiveTopology::TriangleList;
            batch.State.VertexLayouts = {StaticMesh::StaticGetVertexLayout()};
            batch.IsUsedIndices = true;
            batch.IsCastShadow = true;

            batch.Material  = nullptr;
            batch.Priority  = (matIndex < (int)m_priorities.size()) ? m_priorities.at(matIndex) : 0;

            auto& matProxy = m_materialProxies.at(matIndex);
            if (!matProxy)
            {
                // no valid material even after the component-side fallback
                m_batches.pop_back();
                continue;
            }
            batch.Material = matProxy;

            batch.RenderObjectIndex = m_renderObjectIndex;
            batch.ExtraDescriptorSet = m_dummyExtraSet;
            if (m_dummyExtraSet)
                batch.DescriptorSetLayout = m_dummyExtraSet->GetDescriptorSetLayout();

            if (matIndex < vertBuffers.size())
            {
                auto& element = batch.Elements.emplace_back();
                element.Vertex =  vertBuffers[matIndex];
                element.Indices = indexBuffers[matIndex];
                // PerRenderObject data is in global dynamic UBO, no per-element descriptor set needed
            }
        }
    }

    void StaticMeshRenderObject::OnEnterScene()
    {
        if (m_pPerRenderObjectDataManager)
            m_dummyExtraSet = m_pPerRenderObjectDataManager->GetDummyExtraSet();

        SubmitChange();
    }



    SPtr<rendering::SceneProxy> StaticMeshRendererComponent::CreateSceneProxy()
    {
        auto ro = mksptr(new StaticMeshRenderObject());
        if (m_staticMesh)
        {
            m_staticMesh->EnsureGPUResource();
            ro->SetMeshProxy(m_staticMesh->GetMeshProxy())
                ->SetMaterialProxies(ResolveMaterialProxies(*m_materials), *m_priorities);
            // batches are built on the render thread (OnEnterScene -> SubmitChange),
            // after the queued mesh-proxy Init has drained.
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
        // base::BeginComponent created + registered m_proxy via CreateSceneProxy().
        m_renderObject = sptr_static_cast<StaticMeshRenderObject>(m_proxy);
        ResizeMaterials(m_materials->size());

        m_canDrawGizmo = true;
        GetWorld()->GetGizmosManager().AddGizmoComponent(self_ptr());

        OnTransformChanged();
    }
    void StaticMeshRendererComponent::EndComponent()
    {
        if (m_canDrawGizmo)
        {
            GetWorld()->GetGizmosManager().RemoveGizmoComponent(self_ptr());
        }

        m_renderObject.reset();
        // base::EndComponent removes + drops m_proxy.
        base::EndComponent();
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

            // Sparse sampling: skip vertices when there are too many
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

    void StaticMeshRendererComponent::GetDependenciesAsset(array_list<guid_t>& deps) const
    {
        base::GetDependenciesAsset(deps);
        if (m_staticMesh)
        {
            deps.push_back(m_staticMesh.GetGuid());
        }
        if (m_materials)
        {
            for (auto& mat : *m_materials)
            {
                if (mat)
                {
                    deps.push_back(mat.GetGuid());
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
        MarkRenderStateDirty();
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

        MarkRenderStateDirty();
        RebuildObserver();
    }
    void StaticMeshRendererComponent::OnMaterialChanged()
    {
        MarkRenderStateDirty();
        RebuildObserver();
    }

    void StaticMeshRendererComponent::ResolveRenderStateDirty()
    {
        if (!m_renderObject)
            return;

        Matrix4f localToWorld = GetNode()->GetTransform()->GetLocalToWorldMatrix();
        BoxSphereBounds3f bounds = GetBoundsWS();
        array_list<int32_t> priorities = *m_priorities;

        // Resolve assets to render proxies here on the game thread (ensures GPU
        // resources, Error-material fallback); the render object never sees assets.
        std::shared_ptr<rendering::MeshProxy> meshProxy;
        if (m_staticMesh)
        {
            m_staticMesh->EnsureGPUResource();
            meshProxy = m_staticMesh->GetMeshProxy();
        }
        auto materialProxies = ResolveMaterialProxies(*m_materials);

        auto ro = m_renderObject;
        Application::GetRenderThread()->EnqueueUpdate_AnyThread(
            [ro, localToWorld, bounds, meshProxy = std::move(meshProxy), materialProxies = std::move(materialProxies), priorities = std::move(priorities)](gfx::GFXResourceRegistry*) mutable
            {
                ro->SetTransform(localToWorld);
                ro->SetBounds(bounds);
                ro->SetMeshProxy(std::move(meshProxy))->SetMaterialProxies(materialProxies, priorities)->SubmitChange();
            });
    }

} // namespace pulsar
