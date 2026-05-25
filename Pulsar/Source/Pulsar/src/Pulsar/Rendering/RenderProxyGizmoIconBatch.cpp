#include "RenderProxyGizmoIconBatch.h"
#include "Application.h"
#include "Assets/StaticMesh.h"
#include <Pulsar/Rendering/PerRenderObjectDataManager.h>
#include <gfx/GFXApplication.h>

namespace pulsar
{
    static gfx::GFXDescriptorSetLayout_wp s_sharedLayout{};

    void RenderProxyGizmoIconBatch::SetMesh(const RCPtr<StaticMesh>& mesh)
    {
        m_mesh = mesh;
        m_dirty = true;
    }

    void RenderProxyGizmoIconBatch::SetItems(const array_list<IconItem>& items)
    {
        m_items = items;
        m_dirty = true;
    }

    void RenderProxyGizmoIconBatch::InitRHI()
    {
        m_dummyExtraSet = RenderThread::Get().GetPerObjectDataManager().GetDummyExtraSet();

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
        m_dirty = true;
    }

    void RenderProxyGizmoIconBatch::ReleaseRHI()
    {
        for (uint32_t slot : m_itemSlots)
            {
                RenderThread::Get().GetPerObjectDataManager().FreeSlot(slot);
            }
        m_itemSlots.clear();
        m_batches.clear();
        m_descriptorSetLayout.reset();
    }

    void RenderProxyGizmoIconBatch::_Rebuild()
    {
        m_batches.clear();
        if (!m_mesh || m_items.empty())
        {
            m_dirty = false;
            return;
        }

        if (!m_mesh->IsCreatedGPUResource())
            m_mesh->CreateGPUResource();

        auto vertBuffers = m_mesh->GetGPUResourceVertexBuffers();
        auto indicesBuffers = m_mesh->GetGPUResourceIndicesBuffers();
        if (vertBuffers.empty())
        {
            m_dirty = false;
            return;
        }

        // Ensure enough slots
        while (m_itemSlots.size() < m_items.size())
        {
            m_itemSlots.push_back(RenderThread::Get().GetPerObjectDataManager().AllocSlot());
        }

        for (size_t i = 0; i < m_items.size(); ++i)
        {
            auto& item = m_items[i];
            uint32_t slot = m_itemSlots[i];

            PerRenderObjectData data{};
            data.LocalToWorldMatrix = item.Transform;
            data.WorldToLocalMatrix = Inverse(item.Transform);
            data.NormalLocalToWorldMatrix = Transpose(data.WorldToLocalMatrix);
            data.NodePosition = Vector4f{item.Transform[3][0], item.Transform[3][1], item.Transform[3][2], 1.0f};
            data.ShaderFlags = 0;
            RenderThread::Get().GetPerObjectDataManager().SetData(slot, data);

            rendering::MeshBatch batch{};
            batch.Material = item.Material;
            batch.Interface = "RENDERER_STATICMESH";
            batch.DescriptorSetLayout = m_descriptorSetLayout;
            batch.RenderObjectIndex = slot;
            batch.ExtraDescriptorSet = m_dummyExtraSet;
            batch.State.VertexLayouts = {StaticMesh::StaticGetVertexLayout()};
            batch.IsDepthTestDisabled = true;

            auto& element = batch.Elements.emplace_back();
            element.Vertex = vertBuffers[0];
            element.Indices = indicesBuffers.empty() ? gfx::BufferHandle{} : indicesBuffers[0];
            batch.IsUsedIndices = element.Indices.IsValid();

            m_batches.push_back(std::move(batch));
        }

        m_dirty = false;
    }

    array_list<rendering::MeshBatch> RenderProxyGizmoIconBatch::GetMeshBatches()
    {
        if (m_dirty)
            _Rebuild();

        for (auto& batch : m_batches)
        {
            if (batch.Material && !batch.Material->IsCreatedGPUResource())
                batch.Material->CreateGPUResource();
        }
        return m_batches;
    }
}
