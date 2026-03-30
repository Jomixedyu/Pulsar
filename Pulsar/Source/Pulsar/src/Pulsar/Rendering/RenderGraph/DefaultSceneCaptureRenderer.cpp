#include "DefaultSceneCaptureRenderer.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Node.h>
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/World.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXGraphicsPipelineManager.h>
#include "../PerPassResources.h"
#include "../LightingData.h"

namespace pulsar
{
    DefaultSceneCaptureRenderer::DefaultSceneCaptureRenderer()
    {
        m_perPassResources.Initialize();
    }

    DefaultSceneCaptureRenderer::~DefaultSceneCaptureRenderer()
    {
        m_perPassResources.Destroy();
    }

    void DefaultSceneCaptureRenderer::Render(RenderGraph& graph, const RenderCaptureContext& ctx)
    {
        auto* cam   = dynamic_cast<CameraComponent*>(ctx.capture);
        auto* world = ctx.world;

        if (!cam || !world)
        {
            Logger::Log("DefaultSceneCaptureRenderer: invalid capture context (cam or world is null)", LogLevel::Error);
            return;
        }

        auto* camRenderTexture = cam->GetRenderTexture().GetPtr();
        if (!camRenderTexture)
        {
            Logger::Log("DefaultSceneCaptureRenderer: camera has no render texture", LogLevel::Error);
            return;
        }

        auto* perPass = &m_perPassResources;

        PerPassCameraData camData{};
        camData.MatrixV     = cam->GetViewMat();
        camData.MatrixP     = cam->GetProjectionMat();
        camData.MatrixVP    = camData.MatrixP * camData.MatrixV;
        camData.InvMatrixV  = jmath::Inverse(camData.MatrixV);
        camData.InvMatrixP  = jmath::Inverse(camData.MatrixP);
        camData.InvMatrixVP = jmath::Inverse(camData.MatrixVP);
        camData.CamPosition = Vector4f(cam->GetNode()->GetTransform()->GetWorldPosition(), 1.f);
        camData.CamNear     = cam->GetNear();
        camData.CamFar      = cam->GetFar();
        camData.Resolution  = cam->GetRenderTexture()->GetSize2df();
        perPass->UpdateCamera(camData);

        {
            PerPassLightsBufferData lightsData{};
            auto* lightMgr = ctx.world->GetLightManager();
            int lightCount = std::min(lightMgr->GetLightCount(), 63);
            for (int i = 0; i < lightCount; ++i)
            {
                auto& src = lightMgr->GetLightParameter(i);
                auto& dst = lightsData.Lights[i];
                dst.Position  = {src.WorldPosition.x, src.WorldPosition.y, src.WorldPosition.z};
                dst.CutOff    = src.SpotAngles.x;
                dst.Direction = {src.Direction.x, src.Direction.y, src.Direction.z};
                dst.Radius    = src.SourceRadius;
                dst.Color     = src.Color;
            }
            lightsData.PointCount = static_cast<uint32_t>(lightCount);
            perPass->UpdateLights(lightsData);
        }

        perPass->Submit();

        RGTextureHandle hFinal = graph.ImportTexture("FinalOutput", camRenderTexture);

        graph.AddPass("BasePass")
            .Write(hFinal, RGAttachmentDesc{
                .colorLoadOp  = gfx::GFXRenderPassLoadOp::Clear,
                .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                .clearColor   = {0.3f, 0.3f, 0.3f, 1.f},
                .depthLoadOp  = gfx::GFXRenderPassLoadOp::Clear,
                .depthStoreOp = gfx::GFXRenderPassStoreOp::DontCare,
                .clearDepth   = 1.f,
            })
            .WithPerPass(perPass)
            .Execute([cam, world, perPass](RGPassContext& ctx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                auto* targetFBO = cam->GetRenderTexture()->GetGfxFrameBufferObject().get();
                if (!targetFBO) return;

                auto* pipelineMgr   = cmdBuffer.GetApplication()->GetGraphicsPipelineManager();
                auto& renderObjects = world->GetRenderObjects();

                cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

                struct BatchEntry
                {
                    rendering::RenderObject* renderObject;
                    rendering::MeshBatch     batch;
                };

                const Vector3f camPos     = cam->GetNode()->GetTransform()->GetWorldPosition();
                const Vector3f camForward = cam->GetNode()->GetTransform()->GetForward();

                array_list<BatchEntry> opaqueList, alphaTestList, transparentList;

                for (const rendering::RenderObject_sp& ro : renderObjects)
                {
                    const float depth = jmath::Dot(camForward, ro->GetWorldPosition() - camPos);
                    for (auto batch : ro->GetMeshBatches())
                    {
                        batch.Depth = depth;
                        switch (batch.Queue)
                        {
                        case ShaderPassRenderQueueType::AlphaTest:
                            alphaTestList.push_back({ro.get(), std::move(batch)});
                            break;
                        case ShaderPassRenderQueueType::Transparency:
                            transparentList.push_back({ro.get(), std::move(batch)});
                            break;
                        default:
                            opaqueList.push_back({ro.get(), std::move(batch)});
                            break;
                        }
                    }
                }

                auto sortAsc  = [](const BatchEntry& a, const BatchEntry& b)
                {
                    if (a.batch.Priority != b.batch.Priority) return a.batch.Priority < b.batch.Priority;
                    return a.batch.Depth < b.batch.Depth;
                };
                auto sortDesc = [](const BatchEntry& a, const BatchEntry& b)
                {
                    if (a.batch.Priority != b.batch.Priority) return a.batch.Priority < b.batch.Priority;
                    return a.batch.Depth > b.batch.Depth;
                };
                std::sort(opaqueList.begin(),      opaqueList.end(),      sortAsc);
                std::sort(alphaTestList.begin(),   alphaTestList.end(),   sortAsc);
                std::sort(transparentList.begin(), transparentList.end(), sortDesc);

                auto DrawBatchList = [&](const array_list<BatchEntry>& entries)
                {
                    for (const auto& entry : entries)
                    {
                        const auto& batch = entry.batch;
                        if (!batch.Material) continue;
                        auto shader = batch.Material->GetShader();
                        if (!shader || !shader->GetConfig()) continue;

                        const auto& passBinding = batch.Material->GetPassBinding("Forward", batch.Interface);
                        batch.Material->SubmitParameters();
                        auto program = passBinding.GetCurrentProgram();
                        if (!program) continue;

                        auto& gpuPrograms = program->GetGpuPrograms();
                        auto shaderConfig = shader->GetConfig();

                        gfx::GFXGraphicsPipelineStateParams psoParams{};
                        if (shaderConfig->Passes && shaderConfig->Passes->size() > 0)
                        {
                            auto& passConfig = (*shaderConfig->Passes)[0];
                            if (passConfig->GraphicsPipeline)
                            {
                                auto& gp = passConfig->GraphicsPipeline;
                                psoParams.CullMode          = gp->CullMode;
                                psoParams.DepthCompareOp    = gp->ZTestOp;
                                psoParams.DepthWriteEnable  = gp->ZWriteEnabled;
                                psoParams.DepthTestEnable   = true;
                                psoParams.StencilTestEnable = gp->Stencil_Enabled;
                            }
                        }

                        array_list<gfx::GFXDescriptorSetLayout_sp> descLayouts;
                        auto matDescSetLayout = batch.Material->GetDescriptorSetLayout();
                        if (!matDescSetLayout) continue;
                        descLayouts.push_back(matDescSetLayout);
                        descLayouts.push_back(perPass->GetDescriptorSetLayout());
                        descLayouts.push_back(batch.DescriptorSetLayout);

                        auto gfxPipeline = pipelineMgr->GetGraphicsPipeline(
                            gpuPrograms, psoParams, descLayouts,
                            targetFBO->GetRenderTargetDesc(), batch.State);

                        cmdBuffer.CmdBindGraphicsPipeline(gfxPipeline.get());
                        cmdBuffer.CmdSetCullMode(batch.GetCullMode());

                        for (const auto& element : batch.Elements)
                        {
                            array_list<gfx::GFXDescriptorSet*> descSets;
                            descSets.push_back(batch.Material->GetDescriptorSet().get());
                            descSets.push_back(perPass->GetDescriptorSet().get());
                            descSets.push_back(element.ModelDescriptor.get());
                            cmdBuffer.CmdBindDescriptorSets(descSets, gfxPipeline.get());

                            cmdBuffer.CmdBindVertexBuffers({element.Vertex.get()});
                            if (batch.IsUsedIndices)
                            {
                                cmdBuffer.CmdBindIndexBuffer(element.Indices.get());
                                cmdBuffer.CmdDrawIndexed(element.Indices->GetElementCount());
                            }
                            else
                            {
                                cmdBuffer.CmdDraw(element.Vertex->GetElementCount());
                            }
                        }
                    }
                };

                DrawBatchList(opaqueList);
                DrawBatchList(alphaTestList);
                DrawBatchList(transparentList);
            });
    }

} // namespace pulsar
