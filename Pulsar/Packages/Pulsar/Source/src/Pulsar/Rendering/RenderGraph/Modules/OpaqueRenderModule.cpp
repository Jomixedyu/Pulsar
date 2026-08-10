#include "OpaqueRenderModule.h"
#include <Pulsar/Scene.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Rendering/RenderScene.h>
#include <Pulsar/Rendering/SceneView.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <Pulsar/Assets/Material.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>

namespace pulsar
{
    void OpaqueRenderModule::OnRecord(RenderGraph& graph, RenderFrameData& frameData)
    {
        auto* sceneTarget = frameData.Get<SceneTargetFrameData>();
        if (!sceneTarget)
            return;

        auto ctx = MakeRenderCaptureContext(frameData);
        sceneTarget->Target = RecordOpaque(graph, sceneTarget->Target, sceneTarget->Target, ctx);
        
    }

    RGTextureHandle OpaqueRenderModule::RecordOpaque(RenderGraph& graph,
                                           RGTextureHandle input,
                                           RGTextureHandle output,
                                           const RenderCaptureContext& ctx)
    {
        auto* scene = ctx.scene;
        if (!scene || !ctx.view)
            return output;

        const Vector3f camPos     = ctx.view->CameraPosition;
        const Vector3f camForward = ctx.view->CameraForward;
        SceneView* viewProxy      = ctx.viewProxy;

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
            .Prepare([scene, camPos, camForward, preparedOpaque, preparedAlphaTest, this](RGPassContext&)
            {
                for (const rendering::RenderObject_sp& ro : scene->GetRenderObjects())
                {
                    const float depth = jmath::Dot(camForward, ro->GetWorldPosition() - camPos);
                    for (auto batch : ro->GetMeshBatches())
                    {
                        batch.Depth = depth;
                        if (!batch.Material)
                            continue;

                        auto resolved = batch.Material->ResolveRenderVariant("Forward", batch.Interface);

                        auto queue = batch.Material->GetQueue();
                        PreparedBatch pb{ std::move(batch) };
                        if (resolved)
                        {
                            pb.program = resolved.m_program;
                            pb.set0 = resolved.m_set0;
                            pb.set0Layout = resolved.m_set0Layout;
                        }
                        switch (queue)
                        {
                        case ShaderPassRenderQueueType::AlphaTest:
                            preparedAlphaTest->push_back(std::move(pb));
                            break;
                        case ShaderPassRenderQueueType::Transparency:
                            // Transparency is handled by TranslucencyRenderModule
                            break;
                        case ShaderPassRenderQueueType::Overlay:
                            // Overlay batches are drawn after post-processing in GizmoOverlayRenderModule
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
            .Execute([scene, viewProxy, preparedOpaque, preparedAlphaTest, this]
                     (RGPassContext& ctx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                auto* targetFBO = cmdBuffer.GetFrameBuffer();
                if (!targetFBO) return;

                auto* pipelineMgr = cmdBuffer.GetApplication()->GetGraphicsPipelineManager();
                cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

                RenderResourceRegistry reg;
                reg.Set("CameraBuffer", viewProxy ? viewProxy->GetCameraBuffer() : nullptr);
                reg.Set("WorldBuffer", scene->GetWorldBuffer());
                reg.Set("LightBuffer", scene->GetLightsBuffer());
                reg.Set("RenderObjectBuffer", scene->GetPerRenderObjectData().GetBuffer());

                auto getEffectiveGP = [](const PreparedBatch& pb) -> SPtr<ShaderConfigGraphicsPipeline>
                {
                    auto shaderConfig = pb.batch.Material->GetShaderConfig();
                    if (shaderConfig->Passes && shaderConfig->Passes->size() > 0)
                    {
                        auto& passConfig = (*shaderConfig->Passes)[0];
                        return pb.batch.Material->GetEffectiveGraphicsPipeline(passConfig->Name);
                    }
                    return nullptr;
                };

                DrawPreparedBatchList(cmdBuffer, *preparedOpaque, reg,
                                      pipelineMgr, targetFBO->GetRenderTargetDesc(), getEffectiveGP);
                DrawPreparedBatchList(cmdBuffer, *preparedAlphaTest, reg,
                                      pipelineMgr, targetFBO->GetRenderTargetDesc(), getEffectiveGP);
            });

        return output;
    }
}
