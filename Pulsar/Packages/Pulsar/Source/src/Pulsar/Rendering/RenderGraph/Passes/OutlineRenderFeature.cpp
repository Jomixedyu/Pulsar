#include "OutlineRenderFeature.h"
#include <Pulsar/Scene.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Rendering/RenderScene.h>
#include <Pulsar/Rendering/SceneView.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <Pulsar/Rendering/ShaderConfig.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Assets/Shader.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>

namespace pulsar
{
    static bool MaterialHasOutlineRenderFeature(const MaterialProxy* material)
    {
        if (!material)
            return false;
        return material->HasPass("VertexOutline");
    }

    void OutlineRenderFeature::OnRecord(RenderGraph& graph, RenderFrameData& frameData)
    {
        auto* sceneTarget = frameData.Get<SceneTargetFrameData>();
        if (!sceneTarget)
            return;

        auto ctx = MakeRenderCaptureContext(frameData);
        auto* scene = ctx.scene;
        if (!scene || !ctx.view)
            return;

        const Vector3f camPos     = ctx.view->CameraPosition;
        const Vector3f camForward = ctx.view->CameraForward;
        SceneView* viewProxy      = ctx.viewProxy;

        auto preparedOutline = std::make_shared<array_list<PreparedBatch>>();

        graph.AddPass("OutlineRenderFeature")
            .Write(sceneTarget->Target, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                .depthLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .depthStoreOp = gfx::GFXRenderPassStoreOp::Store,
            })
            .Prepare([scene, camPos, camForward, preparedOutline, this](RGPassContext&)
            {
                for (const rendering::RenderObject_sp& ro : scene->GetRenderObjects())
                {
                    const float depth = jmath::Dot(camForward, ro->GetWorldPosition() - camPos);
                    for (auto batch : ro->GetMeshBatches())
                    {
                        if (!batch.Material)
                            continue;

                        auto queue = batch.Material->GetQueue();
                        if (queue != ShaderPassRenderQueueType::Opaque
                            && queue != ShaderPassRenderQueueType::AlphaTest)
                        {
                            continue;
                        }

                        if (!MaterialHasOutlineRenderFeature(batch.Material.get()))
                            continue;

                        batch.Depth = depth;
                        ResolvedVariant resolved;
                        if (batch.Material)
                            resolved = batch.Material->ResolveRenderVariant("VertexOutline", batch.Interface);

                        if (!resolved)
                            continue;

                        PreparedBatch pb{ std::move(batch) };
                        pb.program = resolved.m_program;
                        pb.set0 = resolved.m_set0;
                        pb.set0Layout = resolved.m_set0Layout;
                        preparedOutline->push_back(std::move(pb));
                    }
                }

                auto sortAsc = [](const PreparedBatch& a, const PreparedBatch& b)
                {
                    if (a.batch.Priority != b.batch.Priority)
                        return a.batch.Priority < b.batch.Priority;
                    return a.batch.Depth < b.batch.Depth;
                };
                std::sort(preparedOutline->begin(), preparedOutline->end(), sortAsc);
            })
            .Execute([scene, viewProxy, preparedOutline, this]
                     (RGPassContext& ctx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                auto* targetFBO = cmdBuffer.GetFrameBuffer();
                if (!targetFBO)
                    return;

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
                    if (auto pass = shaderConfig->FindPass("VertexOutline"))
                        return pass->GraphicsPipeline;
                    return nullptr;
                };

                DrawPreparedBatchList(cmdBuffer, *preparedOutline, reg,
                                      pipelineMgr, targetFBO->GetRenderTargetDesc(), getEffectiveGP);
            });
    }
}
