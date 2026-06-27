#include "GizmoOverlayPass.h"
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Rendering/RenderScene.h>
#include <Pulsar/Rendering/SceneView.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <Pulsar/Assets/Material.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>
#include <Pulsar/Rendering/PerPassResources.h>

namespace pulsar
{
    RGTextureHandle GizmoOverlayPass::AddToGraph(RenderGraph& graph,
                                                 RGTextureHandle input,
                                                 RGTextureHandle output,
                                                 const RenderCaptureContext& ctx,
                                                 PerPassResources* perPass)
    {
        auto* scene = ctx.scene;
        if (!scene || !ctx.view)
            return output;

        const Vector3f camPos     = ctx.view->CameraPosition;
        const Vector3f camForward = ctx.view->CameraForward;

        auto preparedOverlay = std::make_shared<array_list<PreparedBatch>>();

        graph.AddPass("GizmoOverlay")
            .Write(output, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                .depthLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .depthStoreOp = gfx::GFXRenderPassStoreOp::DontCare,
            })
            .WithPerPass(perPass)
            .Prepare([scene, camPos, camForward, preparedOverlay, perPass, this](RGPassContext&)
            {
                perPass->WriteStandardBuffers(this->m_perPassSet.get(), scene->GetPerRenderObjectData().GetBuffer());
                perPass->Submit(this->m_perPassSet.get());

                for (const rendering::RenderObject_sp& ro : scene->GetRenderObjects())
                {
                    const float depth = jmath::Dot(camForward, ro->GetWorldPosition() - camPos);
                    for (auto batch : ro->GetMeshBatches())
                    {
                        if (!batch.Material || batch.Material->GetQueue() != ShaderPassRenderQueueType::Overlay)
                            continue;

                        batch.Depth = depth;
                        const MaterialPassBinding* binding = batch.Material
                            ->PrepareForRendering("Forward", batch.Interface);

                        preparedOverlay->push_back(PreparedBatch{ std::move(batch), binding });
                    }
                }

                auto sortAsc = [](const PreparedBatch& a, const PreparedBatch& b)
                {
                    if (a.batch.Priority != b.batch.Priority) return a.batch.Priority < b.batch.Priority;
                    return a.batch.Depth < b.batch.Depth;
                };
                std::sort(preparedOverlay->begin(), preparedOverlay->end(), sortAsc);
            })
            .Execute([perPass, preparedOverlay, this]
                     (RGPassContext& ctx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                auto* targetFBO = cmdBuffer.GetFrameBuffer();
                if (!targetFBO) return;

                auto* pipelineMgr = cmdBuffer.GetApplication()->GetGraphicsPipelineManager();
                cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

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

                DrawPreparedBatchList(cmdBuffer, *preparedOverlay, this->m_perPassSet.get(),
                                      pipelineMgr, targetFBO->GetRenderTargetDesc(), getEffectiveGP);
            });

        return output;
    }
}
