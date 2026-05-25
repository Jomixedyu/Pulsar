#include "RenderProxyGizmoIconBatch.h"
#include "Application.h"
#include "Assets/StaticMesh.h"
#include <Pulsar/Rendering/PerRenderObjectDataManager.h>
#include <Pulsar/Rendering/RenderProxyMaterial.h>
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

        // Create GPU buffers from StaticMesh CPU data
        if (m_mesh)
        {
            auto& cmdList = Application::GetGfxApp()->GetImmediateCommandList();
            for (auto& section : m_mesh->GetSections())
            {
                auto interleavedVerts = section.BuildInterleavedVertices();
                const size_t vertSize = interleavedVerts.size() * sizeof(StaticMeshVertex);

                {
                    gfx::GFXBufferDesc vertexDesc{};
                    vertexDesc.Usage       = gfx::GFXBufferUsage::Vertex;
                    vertexDesc.StorageType = gfx::GFXBufferMemoryPosition::DeviceLocal;
                    vertexDesc.BufferSize  = vertSize;
                    vertexDesc.ElementSize = sizeof(StaticMeshVertex);

                    auto vertBuffer = cmdList.CreateBuffer(vertexDesc);
                    cmdList.UploadBuffer(vertBuffer.Get(), interleavedVerts.data(), vertSize);
                    m_vertexBuffers.push_back(vertBuffer);
                }

                {
                    gfx::GFXBufferDesc indicesDesc{};
                    indicesDesc.Usage       = gfx::GFXBufferUsage::Indices;
                    indicesDesc.StorageType = gfx::GFXBufferMemoryPosition::DeviceLocal;
                    indicesDesc.BufferSize  = section.GetIndicesAllocSize();
                    indicesDesc.ElementSize = sizeof(MeshIndicesType);

                    auto indicesBuffer = cmdList.CreateBuffer(indicesDesc);
                    cmdList.UploadBuffer(indicesBuffer.Get(), section.Indices.data(), section.GetIndicesAllocSize());
                    m_indicesBuffers.push_back(indicesBuffer);
                }
            }
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
        m_vertexBuffers.clear();
        m_indicesBuffers.clear();
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

        if (m_vertexBuffers.empty())
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
            batch.ProxyMaterial = mksptr(new RenderProxyMaterial(item.Material));
            batch.Interface = "RENDERER_STATICMESH";
            batch.DescriptorSetLayout = m_descriptorSetLayout;
            batch.RenderObjectIndex = slot;
            batch.ExtraDescriptorSet = m_dummyExtraSet;
            batch.State.VertexLayouts = {StaticMesh::StaticGetVertexLayout()};
            batch.IsDepthTestDisabled = true;

            auto& element = batch.Elements.emplace_back();
            element.Vertex = m_vertexBuffers[0];
            element.Indices = m_indicesBuffers.empty() ? gfx::BufferHandle{} : m_indicesBuffers[0];
            batch.IsUsedIndices = element.Indices.IsValid();

            m_batches.push_back(std::move(batch));
        }

        m_dirty = false;
    }

    array_list<rendering::MeshBatch> RenderProxyGizmoIconBatch::GetMeshBatches()
    {
        if (m_dirty)
            _Rebuild();

        return m_batches;
    }
}
