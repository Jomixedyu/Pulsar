#include "Application.h"
#include "AssetManager.h"
#include "Assets/StaticMesh.h"
#include "Rendering/RenderThread.h"

#include <Pulsar/Rendering/LineRenderObject.h>
#include <gfx/GFXResourceRegistry.h>

namespace pulsar
{
    // Layout for set2 (dummy, kept for pipeline compatibility)
    static inline gfx::GFXDescriptorSetLayout_wp s_dummyLayout{};

    LineRenderObject::LineRenderObject()
    {
        // Runs on the game thread (creation site), so the asset system is safe here.
        // EnsureGPUResource creates and feeds the render proxy; GetRenderProxy alone
        // returns null if nobody ensured it.
        if (auto mat = AssetManager::Get()->LoadAsset<Material>("Pulsar/Materials/VertexColor"))
        {
            mat->EnsureGPUResource();
            m_materialProxy = mat->GetRenderProxy();
        }
    }

    void LineRenderObject::SetPoints(const array_list<Vector3f>& pointPairs, const array_list<Color4b>& pointColors)
    {
        array_list<StaticMeshVertex> verties;
        verties.reserve(pointPairs.size());
        for (size_t i = 0; i < pointPairs.size(); i++)
        {
            StaticMeshVertex vert{};
            vert.Color = pointColors[i];
            vert.Position = pointPairs[i];

            verties.push_back(vert);
        }
        SetVerties(verties);
    }

    void LineRenderObject::SetVerties(const array_list<StaticMeshVertex>& verties)
    {
        // Game-thread entry: defer the mutation to the render thread so it never races
        // with recording. The SPtr capture keeps the proxy alive until it runs.
        Application::GetRenderThread()->EnqueueUpdate_AnyThread(
            [self = std::static_pointer_cast<LineRenderObject>(shared_from_this()), verties](gfx::GFXResourceRegistry*) mutable
            {
                self->ApplyVerties(std::move(verties));
            });
    }

    void LineRenderObject::ApplyVerties(array_list<StaticMeshVertex> verties)
    {
        m_verties = std::move(verties);
        Fill();
        // Buffer may have grown inside Fill(); keep the cached batch in sync. The buffer is
        // never shrunk, so DrawCount pins the draw to the live vertex count and prevents
        // stale trailing vertices (e.g. leftover lines after switching scenes).
        if (!m_batchs.empty() && !m_batchs[0].Elements.empty())
        {
            m_batchs[0].Elements[0].Vertex = m_vertBuffer;
            m_batchs[0].Elements[0].DrawCount = static_cast<uint32_t>(m_verties.size());
        }
    }
    void LineRenderObject::Fill()
    {
        if (m_vertBuffer)
        {
            auto* buffer = m_vertBuffer.get();
            // Only grow: the buffer is reused across frames and the actual draw count comes
            // from the batch's DrawCount, so a larger-than-needed buffer is harmless.
            if (buffer && sizeof(StaticMeshVertex) * m_verties.size() > buffer->GetSize())
            {
                m_vertBuffer.reset();
            }
        }

        if (!m_vertBuffer && !m_verties.empty())
        {
            gfx::GFXBufferDesc desc{};
            desc.Usage        = gfx::GFXBufferUsage::Vertex;
            // Gizmo lines are rebuilt every frame from the CPU; device-local storage routes
            // through the batched frame-head transfer queue (no sync points).
            desc.StorageType  = gfx::GFXBufferMemoryPosition::DeviceLocal;
            desc.BufferSize   = m_verties.size() * sizeof(StaticMeshVertex);
            desc.ElementSize  = sizeof(StaticMeshVertex);

            m_vertBuffer = Application::GetGfxApp()->GetResourceRegistry()->CreateBuffer(desc);
        }

        if (m_vertBuffer && !m_verties.empty())
        {
            // The buffer is grow-only and may be larger than the vertex array: upload
            // only the live range, not the buffer capacity.
            m_vertBuffer->Update(m_verties.data(), m_verties.size() * sizeof(StaticMeshVertex));
        }
    }

    void LineRenderObject::OnEnterScene()
    {
        base::OnEnterScene();
        if (m_pPerRenderObjectDataManager)
            m_dummyExtraSet = m_pPerRenderObjectDataManager->GetDummyExtraSet();

        if (s_dummyLayout.expired())
        {
            gfx::GFXDescriptorLayoutDesc info{};
            m_meshDescriptorSetLayout = Application::GetGfxApp()->GetResourceRegistry()->GetOrCreateDescriptorSetLayout({});
            s_dummyLayout = m_meshDescriptorSetLayout;
        }
        else
        {
            m_meshDescriptorSetLayout = s_dummyLayout.lock();
        }

        if (!m_vertBuffer && !m_verties.empty())
        {
            gfx::GFXBufferDesc vertexBufferDesc{};
            vertexBufferDesc.Usage       = gfx::GFXBufferUsage::Vertex;
            vertexBufferDesc.StorageType = gfx::GFXBufferMemoryPosition::DeviceLocal;
            vertexBufferDesc.BufferSize  = m_verties.size() * sizeof(StaticMeshVertex);
            vertexBufferDesc.ElementSize = sizeof(StaticMeshVertex);

            m_vertBuffer = Application::GetGfxApp()->GetResourceRegistry()->CreateBuffer(vertexBufferDesc);
            m_vertBuffer->Update(m_verties.data());
        }

        m_batchs.resize(1);
        rendering::MeshBatch& batch = m_batchs[0];
        batch.DescriptorSetLayout = m_meshDescriptorSetLayout;
        batch.RenderObjectIndex = m_renderObjectIndex;
        batch.ExtraDescriptorSet = m_dummyExtraSet;
        batch.Elements.resize(1);
        batch.Elements[0].Vertex = m_vertBuffer;
        batch.Elements[0].DrawCount = static_cast<uint32_t>(m_verties.size());
        batch.State.Topology = gfx::GFXPrimitiveTopology::LineList;
        batch.State.LineWidth = 1.f;
        batch.State.VertexLayouts = {StaticMesh::StaticGetVertexLayout()};
        batch.IsUsedIndices = false;
        batch.IsDepthTestDisabled = !m_depthTestEnabled;
        batch.Material = m_materialProxy;
        batch.QueueOverride = m_renderQueue;
    }

    void LineRenderObject::OnLeaveScene()
    {
        base::OnLeaveScene();
        if (m_vertBuffer)
        {
            m_vertBuffer.reset();
        }
    }

    void LineRenderObject::OnChangedTransform()
    {
    }

    array_list<rendering::MeshBatch> LineRenderObject::GetMeshBatches()
    {
        return m_batchs;
    }
} // namespace pulsar
