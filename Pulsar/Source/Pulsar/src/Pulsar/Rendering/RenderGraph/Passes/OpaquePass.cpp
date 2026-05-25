#include "OpaquePass.h"
#include <Pulsar/Components/SceneCapture2DComponent.h>
#include <Pulsar/World.h>
#include <Pulsar/Scene.h>
#include <Pulsar/Rendering/RenderProxy.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Node.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>
#include <Pulsar/Rendering/PerPassResources.h>

namespace pulsar
{
    RGTextureHandle OpaquePass::AddToGraph(RenderGraph& graph,
                                           RGTextureHandle input,
                                           RGTextureHandle output,
                                           SceneCapture2DComponent* capture2D,
                                           PerPassResources* perPass)
    {
        auto* world = capture2D->GetWorld();
        if (!world)
            return output;

        auto preparedOpaque      = std::make_shared<array_list<PreparedBatch>>();
        auto preparedAlphaTest   = std::make_shared<array_list<PreparedBatch>>();

        graph.AddPass("Opaque")
            .Write(output, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::Clear,
                .colorStoreOp = m_resolveTargetView ? gfx::GFXRenderPassStoreOp::DontCare : gfx::GFXRenderPassStoreOp::Store,
                .clearColor   = {0.3f, 0.3f, 0.3f, 1.f},
                .depthLoadOp  = gfx::GFXRenderPassLoadOp::Clear,
                .depthStoreOp = gfx::GFXRenderPassStoreOp::Store,
                .clearDepth   = 1.f,
                .resolveTargetView = m_resolveTargetView,
            })
            .WithPerPass(perPass)
            .Prepare([capture2D, world, preparedOpaque, preparedAlphaTest, perPass, this](RGPassContext&)
            {
                perPass->WriteStandardBuffers(this->m_perPassSet.get(), world->GetPerRenderObjectDataManager().GetBuffer());
                perPass->Submit(this->m_perPassSet.get());

                const Vector3f camPos     = capture2D->GetNode()->GetTransform()->GetWorldPosition();
                const Vector3f camForward = capture2D->GetNode()->GetTransform()->GetForward();

                for (const rendering::RenderProxy_sp& ro : world->GetRenderObjects())
                {
                    const float depth = jmath::Dot(camForward, ro->GetWorldPosition() - camPos);
                    for (auto batch : ro->GetMeshBatches())
                    {
                        batch.Depth = depth;
                        const MaterialPassBinding* binding = batch.Material
                            ? batch.Material->PrepareForRendering("Forward", batch.Interface)
                            : nullptr;

                        if (!batch.Material)
                            continue;

                        auto queue = batch.Material->GetQueue();
                        PreparedBatch pb{ std::move(batch), binding };
                        switch (queue)
                        {
                        case ShaderPassRenderQueueType::AlphaTest:
                            preparedAlphaTest->push_back(std::move(pb));
                            break;
                        case ShaderPassRenderQueueType::Transparency:
                            // Transparency is handled by TranslucencyPass
                            break;
                        case ShaderPassRenderQueueType::Overlay:
                            // Overlay batches are drawn after post-processing in GizmoOverlayPass
                            break;
                        default:
                            preparedOpaque->push_back(std::move(pb));
                            break;
                        }
                    }
                }

                auto sortAsc  = [](const PreparedBatch& a, const PreparedBatch& b)
                {
                    if (a.batch.Priority != b.batch.Priority) return a.batch.Priority < b.batch.Priority;
                    return a.batch.Depth < b.batch.Depth;
                };
                std::sort(preparedOpaque->begin(),      preparedOpaque->end(),      sortAsc);
                std::sort(preparedAlphaTest->begin(),   preparedAlphaTest->end(),   sortAsc);
            })
            .Execute([capture2D, perPass, preparedOpaque, preparedAlphaTest, this]
                     (RGPassContext& ctx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                auto* targetFBO = cmdBuffer.GetFrameBuffer();
                if (!targetFBO) return;

                auto* pipelineMgr = cmdBuffer.GetApplication()->GetGraphicsPipelineManager();
                cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

                auto getEffectiveGP = [](const PreparedBatch& pb) -> SPtr<ShaderConfigGraphicsPipeline>
                {
                    auto shaderConfig = pb.batch.Material->GetShader()->GetConfig();
                    if (shaderConfig->Passes && shaderConfig->Passes->size() > 0)
                    {
                        auto& passConfig = (*shaderConfig->Passes)[0];
                        return pb.batch.Material->GetEffectiveGraphicsPipeline(passConfig->Name);
                    }
                    return nullptr;
                };

                DrawPreparedBatchList(cmdBuffer, *preparedOpaque, this->m_perPassSet.get(),
                                      pipelineMgr, targetFBO->GetRenderTargetDesc(), getEffectiveGP);
                DrawPreparedBatchList(cmdBuffer, *preparedAlphaTest, this->m_perPassSet.get(),
                                      pipelineMgr, targetFBO->GetRenderTargetDesc(), getEffectiveGP);
            });

        return output;
    }
}
