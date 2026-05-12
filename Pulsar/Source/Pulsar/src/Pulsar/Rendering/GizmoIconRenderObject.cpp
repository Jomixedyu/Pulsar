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
        batch.State.VertexLayouts = {StaticMesh::StaticGetVertexLayout()};

        if (auto shaderConfig = m_material->GetShader()->GetConfig())
        {
            if (shaderConfig->Passes && !shaderConfig->Passes->empty())
            {
                auto& pass0 = (*shaderConfig->Passes)[0];
                batch.Queue = m_material->GetQueue();
                auto effectiveGP = m_material->GetEffectiveGraphicsPipeline(pass0->Name);
                if (effectiveGP)
                    batch.CullMode = effectiveGP->CullMode;
            }
        }

        if (!m_mesh->IsCreatedGPUResource())
            m_mesh->CreateGPUResource();

        auto vertBuffers = m_mesh->GetGPUResourceVertexBuffers();
        auto indicesBuffers = m_mesh->GetGPUResourceIndicesBuffers();
        if (!vertBuffers.empty())
        {
            auto& element = batch.Elements.emplace_back();
            element.Vertex = vertBuffers[0];
            element.Indices = indicesBuffers.empty() ? nullptr : indicesBuffers[0];
            element.ModelDescriptor = m_descriptorSet;
            batch.IsUsedIndices = element.Indices != nullptr;
        }

        m_batches.push_back(std::move(batch));
    }

    void GizmoIconRenderObject::OnCreateResource()
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

        gfx::GFXBufferDesc desc{};
        desc.Usage = gfx::GFXBufferUsage::ConstantBuffer;
        desc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnDevice;
        desc.BufferSize = sizeof(PerRendererData);
        desc.ElementSize = sizeof(PerRendererData);

        m_constantBuffer = Application::GetGfxApp()->CreateBuffer(desc);
        m_descriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_descriptorSetLayout);
        m_descriptorSet->AddDescriptor("ModelObject", 0)->SetConstantBuffer(m_constantBuffer.get());
        m_descriptorSet->Submit();

        _RebuildBatch();
    }

    void GizmoIconRenderObject::OnDestroyResource()
    {
        m_constantBuffer.reset();
        m_descriptorSet.reset();
        m_batches.clear();
    }

    void GizmoIconRenderObject::OnChangedTransform()
    {
        if (m_constantBuffer)
            m_constantBuffer->Fill(&m_perModelData);
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
