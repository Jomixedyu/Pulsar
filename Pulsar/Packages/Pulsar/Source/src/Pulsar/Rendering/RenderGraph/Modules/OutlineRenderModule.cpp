#include "OutlineRenderModule.h"
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
    static bool MaterialHasVertexOutlinePass(const MaterialProxy* material)
    {
        if (!material)
            return false;
        return material->HasPass("VertexOutline");
    }

    void OutlineRenderModule::OnRecord(RenderGraph& graph, RenderFrameData& frameData)
    {
        auto* sceneTarget = frameData.Get<SceneRenderTargetFrameData>();
        if (!sceneTarget)
            return;

        auto* capture = frameData.Get<SceneCaptureFrameData>();
        if (!capture || !capture->scene || !capture->view)
            return;

        auto* culling = frameData.Get<SceneViewCullingFrameData>();
        if (!culling || !culling->VisibleRenderers)
            return;

        auto* scene = capture->scene;
        const Vector3f camPos     = capture->view->CameraPosition;
        const Vector3f camForward = capture->view->CameraForward;
        SceneView* viewProxy      = capture->viewProxy;

        auto preparedOutline = std::make_shared<array_list<PreparedBatch>>();

        graph.AddPass("Outline")
            .Write(sceneTarget->Target, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                .depthLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .depthStoreOp = gfx::GFXRenderPassStoreOp::Store,
            })
            .Prepare([camPos, camForward, visibleRenderers = culling->VisibleRenderers, preparedOutline, this](RGPassContext&)
            {
                for (const rendering::RenderObject_sp& ro : *visibleRenderers)
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

                        if (!MaterialHasVertexOutlinePass(batch.Material.get()))
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
