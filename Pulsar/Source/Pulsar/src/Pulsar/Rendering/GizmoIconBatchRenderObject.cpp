#include "GizmoIconBatchRenderObject.h"

#include "Application.h"
#include "Assets/StaticMesh.h"
#include <gfx/GFXApplication.h>

namespace pulsar
{
    static gfx::GFXDescriptorSetLayout_wp s_sharedLayout{};

    void GizmoIconBatchRenderObject::SetMesh(const RCPtr<StaticMesh>& mesh)
    {
        m_mesh = mesh;
        m_dirty = true;
    }

    void GizmoIconBatchRenderObject::SetItems(const array_list<IconItem>& items)
    {
        m_items = items;
        m_dirty = true;
    }

    void GizmoIconBatchRenderObject::OnCreateResource()
    {
        if (s_sharedLayout.expired())
        {
            gfx::GFXDescriptorSetLayoutDesc info{
                gfx::GFXDescriptorType::ConstantBuffer,
                gfx::GFXGpuProgramStageFlags::VertexFragment,
                0, kRenderingDescriptorSpace_ModelInfo};
            m_descriptorSetLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(&info, 1);
            s_sharedLayout = m_descriptorSetLayout;
        }
        else
        {
            m_descriptorSetLayout = s_sharedLayout.lock();
        }
        m_dirty = true;
    }

    void GizmoIconBatchRenderObject::OnDestroyResource()
    {
        m_itemGPU.clear();
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

        if (!m_mesh->IsCreatedGPUResource())
            m_mesh->CreateGPUResource();

        auto vertBuffers = m_mesh->GetGPUResourceVertexBuffers();
        auto indicesBuffers = m_mesh->GetGPUResourceIndicesBuffers();
        if (vertBuffers.empty())
        {
            m_dirty = false;
            return;
        }

        // Ensure enough GPU resources
        while (m_itemGPU.size() < m_items.size())
        {
            PerItemGPU gpu{};
            gfx::GFXBufferDesc desc{};
            desc.Usage = gfx::GFXBufferUsage::ConstantBuffer;
            desc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnDevice;
            desc.BufferSize = sizeof(PerRendererData);
            desc.ElementSize = sizeof(PerRendererData);
            gpu.ConstantBuffer = Application::GetGfxApp()->CreateBuffer(desc);
            gpu.DescriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_descriptorSetLayout);
            gpu.DescriptorSet->AddDescriptor("ModelObject", 0)->SetConstantBuffer(gpu.ConstantBuffer.get());
            gpu.DescriptorSet->Submit();
            m_itemGPU.push_back(std::move(gpu));
        }

        for (size_t i = 0; i < m_items.size(); ++i)
        {
            auto& item = m_items[i];
            auto& gpu = m_itemGPU[i];

            PerRendererData data{};
            data.LocalToWorldMatrix = item.Transform;
            data.WorldToLocalMatrix = Inverse(item.Transform);
            data.NormalLocalToWorldMatrix = Transpose(data.WorldToLocalMatrix);
            data.NodePosition = Vector4f{item.Transform[3][0], item.Transform[3][1], item.Transform[3][2], 1.0f};
            data.ShaderFlags = 0;
            gpu.ConstantBuffer->Fill(&data);

            rendering::MeshBatch batch{};
            batch.Material = item.Material;
            batch.Interface = "RENDERER_STATICMESH";
            batch.DescriptorSetLayout = m_descriptorSetLayout;
            batch.State.VertexLayouts = {StaticMesh::StaticGetVertexLayout()};
            batch.Queue = ShaderPassRenderQueueType::Overlay;
            batch.IsDepthTestDisabled = true;

            auto& element = batch.Elements.emplace_back();
            element.Vertex = vertBuffers[0];
            element.Indices = indicesBuffers.empty() ? nullptr : indicesBuffers[0];
            element.ModelDescriptor = gpu.DescriptorSet;
            batch.IsUsedIndices = element.Indices != nullptr;

            if (item.Material && item.Material->GetShader() && item.Material->GetShader()->GetConfig())
            {
                auto shaderConfig = item.Material->GetShader()->GetConfig();
                if (shaderConfig->Passes && !shaderConfig->Passes->empty())
                {
                    auto& pass0 = (*shaderConfig->Passes)[0];
                    batch.Queue = item.Material->GetQueue();
                    auto effectiveGP = item.Material->GetEffectiveGraphicsPipeline(pass0->Name);
                    if (effectiveGP)
                        batch.CullMode = effectiveGP->CullMode;
                }
            }

            m_batches.push_back(std::move(batch));
        }

        m_dirty = false;
    }

    array_list<rendering::MeshBatch> GizmoIconBatchRenderObject::GetMeshBatches()
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
