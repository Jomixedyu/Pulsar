#include "OutlinePass.h"
#include <Pulsar/Components/SceneCapture2DComponent.h>
#include <Pulsar/World.h>
#include <Pulsar/Scene.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <Pulsar/Rendering/ShaderConfig.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Node.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>
#include <Pulsar/Rendering/PerPassResources.h>

namespace pulsar
{
    static bool MaterialHasOutlinePass(const Material* material)
    {
        if (!material)
            return false;
        auto shader = material->GetShader();
        if (!shader)
            return false;
        auto config = shader->GetConfig();
        if (!config || !config->Passes)
            return false;
        for (const auto& pass : *config->Passes)
        {
            if (pass && pass->Name == "VertexOutline")
                return true;
        }
        return false;
    }

    RGTextureHandle OutlinePass::AddToGraph(RenderGraph& graph,
                                            RGTextureHandle input,
                                            RGTextureHandle output,
                                            SceneCapture2DComponent* capture2D,
                                            PerPassResources* perPass)
    {
        auto* world = capture2D->GetWorld();
        if (!world)
            return output;

        auto preparedOutline = std::make_shared<array_list<PreparedBatch>>();

        graph.AddPass("OutlinePass")
            .Write(output, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                .depthLoadOp  = gfx::GFXRenderPassLoadOp::Load,
                .depthStoreOp = gfx::GFXRenderPassStoreOp::Store,
            })
            .WithPerPass(perPass)
            .Prepare([capture2D, world, preparedOutline, perPass, this](RGPassContext&)
            {
                perPass->WriteStandardBuffers(this->m_perPassSet.get());
                perPass->Submit(this->m_perPassSet.get());

                const Vector3f camPos     = capture2D->GetNode()->GetTransform()->GetWorldPosition();
                const Vector3f camForward = capture2D->GetNode()->GetTransform()->GetForward();

                for (const rendering::RenderObject_sp& ro : world->GetRenderObjects())
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

                        if (!MaterialHasOutlinePass(batch.Material.GetPtr()))
                            continue;

                        batch.Depth = depth;
                        const MaterialPassBinding* binding = batch.Material
                            ? batch.Material->PrepareForRendering("VertexOutline", batch.Interface)
                            : nullptr;

                        if (!binding)
                            continue;

                        PreparedBatch pb{ std::move(batch), binding };
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
            .Execute([capture2D, perPass, preparedOutline, this]
                     (RGPassContext& ctx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                auto* targetFBO = cmdBuffer.GetFrameBuffer();
                if (!targetFBO)
                    return;

                auto* pipelineMgr = cmdBuffer.GetApplication()->GetGraphicsPipelineManager();
                cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

                auto getEffectiveGP = [](const PreparedBatch& pb) -> SPtr<ShaderConfigGraphicsPipeline>
                {
                    auto shaderConfig = pb.batch.Material->GetShader()->GetConfig();
                    // Outline pass 直接读 shader 原始配置，不应用 material override
                    if (auto pass = shaderConfig->FindPass("VertexOutline"))
                        return pass->GraphicsPipeline;
                    return nullptr;
                };

                DrawPreparedBatchList(cmdBuffer, *preparedOutline, this->m_perPassSet.get(),
                                      pipelineMgr, targetFBO->GetRenderTargetDesc(), getEffectiveGP);
            });

        return output;
    }
}
