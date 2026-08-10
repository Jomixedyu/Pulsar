#include "GizmoOverlayRenderModule.h"
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
    void GizmoOverlayRenderModule::OnRecord(RenderGraph& graph, RenderFrameData& frameData)
    {
        auto* postProcess = frameData.Get<PostProcessFrameData>();
        if (!postProcess)
            return;

        auto ctx = MakeRenderCaptureContext(frameData);
        RecordOverlayPass(graph, postProcess->FinalTarget, postProcess->FinalTarget, ctx);
    }

    RGTextureHandle GizmoOverlayRenderModule::RecordOverlayPass(RenderGraph& graph,
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

        auto preparedOverlay = std::make_shared<array_list<PreparedBatch>>();

        graph.AddPass("GizmoOverlay")
            .Write(output, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                .depthLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .depthStoreOp = gfx::GFXRenderPassStoreOp::DontCare,
            })
            .Prepare([scene, camPos, camForward, preparedOverlay, this](RGPassContext&)
            {
                for (const rendering::RenderObject_sp& ro : scene->GetRenderObjects())
                {
                    const float depth = jmath::Dot(camForward, ro->GetWorldPosition() - camPos);
                    for (auto batch : ro->GetMeshBatches())
                    {
                        if (!batch.Material || batch.Material->GetQueue() != ShaderPassRenderQueueType::Overlay)
                            continue;

                        batch.Depth = depth;
                        auto resolved = batch.Material->ResolveRenderVariant("Forward", batch.Interface);

                        PreparedBatch pb{ std::move(batch) };
                        if (resolved)
                        {
                            pb.program = resolved.m_program;
                            pb.set0 = resolved.m_set0;
                            pb.set0Layout = resolved.m_set0Layout;
                        }
                        preparedOverlay->push_back(std::move(pb));
                    }
                }

                auto sortAsc = [](const PreparedBatch& a, const PreparedBatch& b)
                {
                    if (a.batch.Priority != b.batch.Priority) return a.batch.Priority < b.batch.Priority;
                    return a.batch.Depth < b.batch.Depth;
                };
                std::sort(preparedOverlay->begin(), preparedOverlay->end(), sortAsc);
            })
            .Execute([scene, viewProxy, preparedOverlay, this]
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
                    // GizmoOverlay pass 直接读 shader 原始配置，不应用 material override
                    if (shaderConfig->Passes && shaderConfig->Passes->size() > 0)
                    {
                        auto& passConfig = (*shaderConfig->Passes)[0];
                        return passConfig->GraphicsPipeline;
                    }
                    return nullptr;
                };

                DrawPreparedBatchList(cmdBuffer, *preparedOverlay, reg,
                                      pipelineMgr, targetFBO->GetRenderTargetDesc(), getEffectiveGP);
            });

        return output;
    }
}
