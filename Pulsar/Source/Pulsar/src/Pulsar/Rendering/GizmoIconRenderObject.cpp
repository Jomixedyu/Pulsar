#include "GizmoIconRenderObject.h"

#include "Application.h"
#include "Assets/StaticMesh.h"
#include <gfx/GFXApplication.h>

namespace pulsar
{
    static gfx::GFXDescriptorSetLayout_wp s_sharedLayout{};

    void GizmoIconRenderObject::SetMesh(const RCPtr<StaticMesh>& mesh)
    {
        m_mesh = mesh;
    }

    void GizmoIconRenderObject::SetMaterial(const RCPtr<Material>& material)
    {
        m_material = material;
    }

    void GizmoIconRenderObject::_RebuildBatch()
    {
        m_batches.clear();
        if (!m_mesh || !m_material)
            return;

        if (!m_material->GetShader())
            return;

        rendering::MeshBatch batch{};
        batch.Material = m_material;
        batch.Interface = "RENDERER_STATICMESH";
        batch.DescriptorSetLayout = m_descriptorSetLayout;
        batch.RenderObjectIndex = m_renderObjectIndex;
        batch.ExtraDescriptorSet = m_dummyExtraSet;
        batch.State.VertexLayouts = {StaticMesh::StaticGetVertexLayout()};

        if (!m_mesh->IsCreatedGPUResource())
            m_mesh->CreateGPUResource();

        auto vertBuffers = m_mesh->GetGPUResourceVertexBuffers();
        auto indicesBuffers = m_mesh->GetGPUResourceIndicesBuffers();
        if (!vertBuffers.empty())
        {
            auto& element = batch.Elements.emplace_back();
            element.Vertex = vertBuffers[0];
            element.Indices = indicesBuffers.empty() ? nullptr : indicesBuffers[0];
            // PerRenderObject data is in global dynamic UBO
            batch.IsUsedIndices = element.Indices != nullptr;
        }

        m_batches.push_back(std::move(batch));
    }

    void GizmoIconRenderObject::OnCreateResource()
    {
        if (m_pPerRenderObjectDataManager)
            m_dummyExtraSet = m_pPerRenderObjectDataManager->GetDummyExtraSet();

        if (s_sharedLayout.expired())
        {
            gfx::GFXDescriptorSetLayoutDesc info{};
            m_descriptorSetLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(&info, 0);
            s_sharedLayout = m_descriptorSetLayout;
        }
        else
        {
            m_descriptorSetLayout = s_sharedLayout.lock();
        }

        _RebuildBatch();
    }

    void GizmoIconRenderObject::OnDestroyResource()
    {
        m_batches.clear();
    }

    void GizmoIconRenderObject::OnChangedTransform()
    {
    }

    array_list<rendering::MeshBatch> GizmoIconRenderObject::GetMeshBatches()
    {
        for (auto& batch : m_batches)
        {
            if (batch.Material && !batch.Material->IsCreatedGPUResource())
                batch.Material->CreateGPUResource();
        }
        return m_batches;
    }
}
