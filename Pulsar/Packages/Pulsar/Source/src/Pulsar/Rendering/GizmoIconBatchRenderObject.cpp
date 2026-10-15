#include "GizmoIconBatchRenderObject.h"

#include "Application.h"
#include "Assets/StaticMesh.h"
#include <Pulsar/Rendering/MaterialProxy.h>
#include <Pulsar/Rendering/MeshProxy.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXResourceRegistry.h>

namespace pulsar
{
    static gfx::GFXDescriptorSetLayout_wp s_sharedLayout{};

    void GizmoIconBatchRenderObject::SetMesh(const std::shared_ptr<rendering::MeshProxy>& mesh)
    {
        m_mesh = mesh;
        m_dirty = true;
    }

    void GizmoIconBatchRenderObject::SetItems(const array_list<IconItem>& items)
    {
        m_items = items;
        m_dirty = true;
    }

    void GizmoIconBatchRenderObject::OnEnterScene()
    {
        if (m_pPerRenderObjectDataManager)
            m_dummyExtraSet = m_pPerRenderObjectDataManager->GetDummyExtraSet();

        if (s_sharedLayout.expired())
        {
            gfx::GFXDescriptorLayoutDesc info{};
            m_descriptorSetLayout = Application::GetGfxApp()->GetResourceRegistry()->GetOrCreateDescriptorSetLayout({});
            s_sharedLayout = m_descriptorSetLayout;
        }
        else
        {
            m_descriptorSetLayout = s_sharedLayout.lock();
        }
        m_dirty = true;
    }

    void GizmoIconBatchRenderObject::OnLeaveScene()
    {
        if (m_pPerRenderObjectDataManager)
        {
            for (uint32_t slot : m_itemSlots)
            {
                m_pPerRenderObjectDataManager->FreeSlot(slot);
            }
        }
        m_itemSlots.clear();
        m_batches.clear();
        m_descriptorSetLayout.reset();
    }

    void GizmoIconBatchRenderObject::_Rebuild()
    {
        m_batches.clear();
        if (!m_mesh || m_items.empty())
        {
            m_dirty = false;
            return;
        }

        // The mesh proxy is prepared by the owner (game thread); this object
        // only ever reads it.
        const auto& vertBuffers = m_mesh->GetVertexBuffers();
        const auto& indicesBuffers = m_mesh->GetIndexBuffers();
        if (vertBuffers.empty())
            return;   // mesh proxy not initialized yet; stay dirty and retry next frame

        if (!m_pPerRenderObjectDataManager)
        {
            m_dirty = false;
            return;
        }

        // Ensure enough slots
        while (m_itemSlots.size() < m_items.size())
        {
            m_itemSlots.push_back(m_pPerRenderObjectDataManager->AllocSlot());
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
            m_pPerRenderObjectDataManager->SetData(slot, data);

            rendering::MeshBatch batch{};
            batch.DescriptorSetLayout = m_descriptorSetLayout;
            batch.RenderObjectIndex = slot;
            batch.ExtraDescriptorSet = m_dummyExtraSet;
            batch.State.VertexLayouts = {StaticMesh::StaticGetVertexLayout()};
            batch.IsDepthTestDisabled = true;

            auto& element = batch.Elements.emplace_back();
            element.Vertex = vertBuffers[0];
            element.Indices = indicesBuffers.empty() ? nullptr : indicesBuffers[0];
            batch.IsUsedIndices = element.Indices != nullptr;

            m_batches.push_back(std::move(batch));
        }

        m_dirty = false;
    }

    array_list<rendering::MeshBatch> GizmoIconBatchRenderObject::GetMeshBatches()
    {
        if (m_dirty)
            _Rebuild();

        for (size_t i = 0; i < m_batches.size() && i < m_items.size(); ++i)
        {
            m_batches[i].Material = m_items[i].MaterialProxy;
        }
        return m_batches;
    }
}
