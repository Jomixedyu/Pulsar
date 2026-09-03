#include "TranslucencyRenderModule.h"
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
    void TranslucencyRenderModule::OnRecord(RenderGraph& graph, RenderFrameData& frameData)
    {
        auto* sceneTarget = frameData.Get<SceneRenderTargetFrameData>();
        if (!sceneTarget)
            return;

        if (auto* opaqueColor = frameData.Get<SceneOpaqueColorFrameData>(); opaqueColor && opaqueColor->Color.IsValid())
            SetOpaqueColor(opaqueColor->Color);

        auto* capture = frameData.Get<SceneCaptureFrameData>();
        if (!capture)
            return;

        auto* culling = frameData.Get<SceneViewCullingFrameData>();
        if (!culling || !culling->VisibleRenderers)
            return;

        sceneTarget->Target = RecordTranslucency(graph, sceneTarget->Target, sceneTarget->Target, *capture, culling->VisibleRenderers);
        
    }

    RGTextureHandle TranslucencyRenderModule::RecordTranslucency(RenderGraph& graph,
                                                 RGTextureHandle input,
                                                 RGTextureHandle output,
                                                 const SceneCaptureFrameData& capture,
                                                 const std::shared_ptr<array_list<rendering::RenderObject_sp>>& visibleRenderers)
    {
        auto* scene = capture.scene;
        if (!scene || !capture.view)
            return output;

        const Vector3f camPos     = capture.view->CameraPosition;
        const Vector3f camForward = capture.view->CameraForward;
        SceneView* viewProxy      = capture.viewProxy;

        auto preparedTransparent = std::make_shared<array_list<PreparedBatch>>();

        auto& passBuilder = graph.AddPass("Translucency")
            .Read(m_hOpaqueColor)
            .Write(output, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                .depthLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .depthStoreOp = gfx::GFXRenderPassStoreOp::DontCare,
            });

        if (m_hOpaqueDepth.IsValid())
        {
            passBuilder.Read(m_hOpaqueDepth);
        }

        passBuilder.Prepare([camPos, camForward, visibleRenderers, preparedTransparent, this](RGPassContext& ctx)
        {
            for (const rendering::RenderObject_sp& ro : *visibleRenderers)
            {
                const float depth = jmath::Dot(camForward, ro->GetWorldPosition() - camPos);
                for (auto batch : ro->GetMeshBatches())
                {
                    if (!batch.Material || batch.Material->GetQueue() != ShaderPassRenderQueueType::Transparency)
                        continue;

                    batch.Depth = depth;
                    auto resolved = batch.Material->ResolveRenderVariant("Forward", batch.VariantFeature);

                    PreparedBatch pb{ std::move(batch) };
                    if (resolved)
                    {
                        pb.program = resolved.m_program;
                        pb.set0 = resolved.m_set0;
                        pb.set0Layout = resolved.m_set0Layout;
                    }
                    preparedTransparent->push_back(std::move(pb));
                }
            }

            auto sortDesc = [](const PreparedBatch& a, const PreparedBatch& b)
            {
                if (a.batch.Priority != b.batch.Priority) return a.batch.Priority < b.batch.Priority;
                return a.batch.Depth > b.batch.Depth;
            };
            std::sort(preparedTransparent->begin(), preparedTransparent->end(), sortDesc);
        })
        .Execute([scene, viewProxy, preparedTransparent, this]
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
            if (auto* colorRT = ctx.Get(m_hOpaqueColor))
            {
                if (auto colorView = colorRT->GetRenderTarget0())
                    reg.Set("PP_InColor", colorView.get());
            }

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

            DrawPreparedBatchList(cmdBuffer, *preparedTransparent, reg,
                                  pipelineMgr, targetFBO->GetRenderTargetDesc(), getEffectiveGP);
        });

        return output;
    }
}
