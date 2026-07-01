#include "TranslucencyPass.h"
#include <Pulsar/Scene.h>
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
    RGTextureHandle TranslucencyPass::AddToGraph(RenderGraph& graph,
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

        auto preparedTransparent = std::make_shared<array_list<PreparedBatch>>();

        auto& passBuilder = graph.AddPass("TranslucencyPass")
            .Read(m_hOpaqueColor)
            .Write(output, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                .depthLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .depthStoreOp = gfx::GFXRenderPassStoreOp::DontCare,
            })
            .WithPerPass(perPass);

        if (m_hOpaqueDepth.IsValid())
        {
            passBuilder.Read(m_hOpaqueDepth);
        }

        passBuilder.Prepare([perPass, camPos, camForward, scene, preparedTransparent, this](RGPassContext& ctx)
        {
            for (const rendering::RenderObject_sp& ro : scene->GetRenderObjects())
            {
                const float depth = jmath::Dot(camForward, ro->GetWorldPosition() - camPos);
                for (auto batch : ro->GetMeshBatches())
                {
                    if (!batch.Material || batch.Material->GetQueue() != ShaderPassRenderQueueType::Transparency)
                        continue;

                    batch.Depth = depth;
                    const MaterialVariant* binding = batch.Material->ResolveRenderVariant("Forward", batch.Interface);

                    PreparedBatch pb{ std::move(batch), binding };
                    preparedTransparent->push_back(std::move(pb));
                }
            }

            auto sortDesc = [](const PreparedBatch& a, const PreparedBatch& b)
            {
                if (a.batch.Priority != b.batch.Priority) return a.batch.Priority < b.batch.Priority;
                return a.batch.Depth > b.batch.Depth;
            };
            std::sort(preparedTransparent->begin(), preparedTransparent->end(), sortDesc);

            // Update per-pass textures for this pass's own descriptor set
            perPass->WriteStandardBuffers(this->m_perPassSet.get(), scene->GetPerRenderObjectData().GetBuffer());

            gfx::GFXTexture2DView* colorView = nullptr;
            if (auto* colorRT = ctx.Get(m_hOpaqueColor))
                colorView = colorRT->GetRenderTarget0().get();
            perPass->WriteTexture(this->m_perPassSet.get(), 3, colorView);

            perPass->Submit(this->m_perPassSet.get());
        })
        .Execute([perPass, preparedTransparent, this]
                 (RGPassContext& ctx, gfx::GFXCommandBuffer& cmdBuffer)
        {

            auto* targetFBO = cmdBuffer.GetFrameBuffer();
            if (!targetFBO) return;

            auto* pipelineMgr = cmdBuffer.GetApplication()->GetGraphicsPipelineManager();
            cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

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

            DrawPreparedBatchList(cmdBuffer, *preparedTransparent, this->m_perPassSet.get(),
                                  pipelineMgr, targetFBO->GetRenderTargetDesc(), getEffectiveGP);
        });

        return output;
    }
}
