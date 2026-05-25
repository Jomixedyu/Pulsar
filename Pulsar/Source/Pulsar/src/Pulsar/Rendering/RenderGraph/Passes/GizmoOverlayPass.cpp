#include "GizmoOverlayPass.h"
#include <Pulsar/Components/SceneCapture2DComponent.h>
#include <Pulsar/World.h>
#include <Pulsar/Rendering/PerRenderObjectDataManager.h>
#include <Pulsar/Rendering/RenderProxyRegistry.h>
#include <Pulsar/Node.h>
#include <Pulsar/Rendering/RenderProxy.h>
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
                                                 SceneCapture2DComponent* capture2D,
                                                 PerPassResources* perPass)
    {
        auto* world = capture2D->GetWorld();
        if (!world)
            return output;

        auto preparedOverlay = std::make_shared<array_list<PreparedBatch>>();

        graph.AddPass("GizmoOverlay")
            .Write(output, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                .depthLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .depthStoreOp = gfx::GFXRenderPassStoreOp::DontCare,
            })
            .WithPerPass(perPass)
            .Prepare([capture2D, world, preparedOverlay, perPass, this](RGPassContext&)
            {
                perPass->WriteStandardBuffers(this->m_perPassSet.get(), world->GetPerRenderObjectDataManager().GetBuffer());
                perPass->Submit(this->m_perPassSet.get());

                const Vector3f camPos     = capture2D->GetNode()->GetTransform()->GetWorldPosition();
                const Vector3f camForward = capture2D->GetNode()->GetTransform()->GetForward();

                for (rendering::RenderProxy* ro : RenderThread::Get().GetProxyRegistry().GetProxies())
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
            .Execute([capture2D, perPass, preparedOverlay, this]
                     (RGPassContext& ctx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                auto* targetFBO = cmdBuffer.GetFrameBuffer();
                if (!targetFBO) return;

                auto* pipelineMgr = cmdBuffer.GetApplication()->GetGraphicsPipelineManager();
                cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

                auto getEffectiveGP = [](const PreparedBatch& pb) -> SPtr<ShaderConfigGraphicsPipeline>
                {
                    auto shaderConfig = pb.batch.Material->GetShader()->GetConfig();
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
