#include "TranslucencyPass.h"
#include <Pulsar/Components/SceneCapture2DComponent.h>
#include <Pulsar/World.h>
#include <Pulsar/Scene.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Node.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>
#include <Pulsar/Rendering/PerPassResources.h>

namespace pulsar
{
    RGTextureHandle TranslucencyPass::AddToGraph(RenderGraph& graph,
                                                 RGTextureHandle input,
                                                 RGTextureHandle output,
                                                 SceneCapture2DComponent* capture2D,
                                                 PerPassResources* perPass)
    {
        auto* world = capture2D->GetWorld();
        if (!world)
            return output;

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

        passBuilder.Prepare([perPass, capture2D, world, preparedTransparent, this](RGPassContext& ctx)
        {
            const Vector3f camPos     = capture2D->GetNode()->GetTransform()->GetWorldPosition();
            const Vector3f camForward = capture2D->GetNode()->GetTransform()->GetForward();

            for (const rendering::RenderObject_sp& ro : world->GetRenderObjects())
            {
                const float depth = jmath::Dot(camForward, ro->GetWorldPosition() - camPos);
                for (auto batch : ro->GetMeshBatches())
                {
                    if (!batch.Material || batch.Material->GetQueue() != ShaderPassRenderQueueType::Transparency)
                        continue;

                    batch.Depth = depth;
                    const MaterialPassBinding* binding = batch.Material->PrepareForRendering("Forward", batch.Interface);

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
            perPass->WriteStandardBuffers(this->m_perPassSet.get(), world->GetPerRenderObjectDataManager().GetBuffer());

            gfx::GFXTexture2DView* colorView = nullptr;
            if (auto* colorRT = ctx.Get(m_hOpaqueColor))
                colorView = colorRT->GetRenderTarget0().get();
            perPass->WriteTexture(this->m_perPassSet.get(), 3, colorView);

            perPass->Submit(this->m_perPassSet.get());
        })
        .Execute([perPass, capture2D, preparedTransparent, this]
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

            DrawPreparedBatchList(cmdBuffer, *preparedTransparent, this->m_perPassSet.get(),
                                  pipelineMgr, targetFBO->GetRenderTargetDesc(), getEffectiveGP);
        });

        return output;
    }
}
