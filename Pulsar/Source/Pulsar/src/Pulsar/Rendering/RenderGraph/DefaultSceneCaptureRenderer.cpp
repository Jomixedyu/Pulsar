#include "DefaultSceneCaptureRenderer.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Node.h>
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Components/SceneCaptureComponent.h>
#include <Pulsar/World.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Application.h>
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

        // ---- Post-Process Passes ----
        // Collect valid post-process materials from the capture component
        const size_t ppCount = ctx.capture ? ctx.capture->GetPostProcessCount() : 0;

        if (ppCount > 0)
        {
            // Build a transient RT with the same size/format as the final output for ping-pong
            auto* camRT  = cam->GetRenderTexture().GetPtr();
            auto* camFBO = camRT->GetGfxFrameBufferObject().get();

            RGTextureDesc pingPongDesc{};
            pingPongDesc.Width  = camRT->GetWidth();
            pingPongDesc.Height = camRT->GetHeight();
            // 后处理 pass 只需要 color attachment，不需要 depth/stencil
            if (camFBO)
            {
                const auto& rtDesc = camFBO->GetRenderTargetDesc();
                for (auto fmt : rtDesc.ColorFormats)
                    pingPongDesc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, fmt });
            }

            RGTextureHandle hPingPong = graph.CreateTransient("PostProcessPingPong", pingPongDesc);

            // hSrc: 当前读取源（初始为 BasePass 的输出 hFinal）
            // hDst: 当前写入目标（初始为 hPingPong）
            // 每个 Pass 结束后交换 hSrc/hDst，绝不在 Read 和 Write 使用同一张纹理。
            // 若最终结果落在 hPingPong（奇数个 Pass），额外追加一个 CopyToFinal Pass。
            RGTextureHandle hSrc = hFinal;
            RGTextureHandle hDst = hPingPong;

            size_t passIdx = 0;
            for (size_t i = 0; i < ppCount; ++i)
            {
                RCPtr<Material> ppMat = ctx.capture->GetPostprocess(i);
                if (!ppMat) continue;

                const size_t curIdx = passIdx++;
                std::string passName = "PostProcessMaterial_" + std::to_string(curIdx);

                // 在 build 阶段提前获取或创建 per-Renderer (set2) 资源（layout+set），
                // 缓存到 m_ppRendererCache，避免在 execute lambda 里每帧动态分配。
                // 以 curIdx 为下标（不以材质指针为 key，避免材质析构后地址复用导致缓存污染）
                if (curIdx >= m_ppRendererCache.size())
                    m_ppRendererCache.resize(curIdx + 1);
                auto& ppRes = m_ppRendererCache[curIdx];
                if (!ppRes.layout)
                {
                    auto* gfxApp = Application::GetGfxApp();
                    gfx::GFXDescriptorSetLayoutDesc desc{
                        gfx::GFXDescriptorType::CombinedImageSampler,
                        gfx::GFXGpuProgramStageFlags::Fragment,
                        0, 1
                    };
                    ppRes.layout = gfxApp->CreateDescriptorSetLayout(&desc, 1);
                    ppRes.set    = gfxApp->GetDescriptorManager()->GetDescriptorSet(ppRes.layout);
                }

                auto* ppRendererSet    = ppRes.set.get();
                auto  ppRendererLayout = ppRes.layout;

                graph.AddPass(passName)
                    .Read(hSrc)
                    .Write(hDst, RGAttachmentDesc{
                        .colorLoadOp  = gfx::GFXRenderPassLoadOp::DontCare,
                        .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                    })
                    .WithPerPass(perPass)
                    .Execute([ppMat, hSrc, hDst, perPass, ppRendererSet, ppRendererLayout](RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
                    {
                        auto* mat = ppMat.GetPtr();
                        if (!mat) return;

                        auto shader = mat->GetShader();
                        if (!shader || !shader->GetConfig()) return;

                        // set 0: per-Material (user params)
                        // set 1: per-Pass (camera / lights)
                        // set 2: per-Renderer — PP_InColor (t0/s0, space2)
                        const auto& ppPassBinding = mat->GetPassBinding("PostProcess", "RENDERER_IMAGEPROCESS");
                        // SubmitParameters 初始化 set0 descriptor set layout
                        mat->SubmitParameters();
                        auto program = ppPassBinding.GetCurrentProgram();
                        if (!program) return;

                        if (!ppRendererSet) return;

                        // ping-pong RT 只有 color，FBO 在 execute 时可以正常取到
                        auto* dstRT = passCtx.Get(hDst);
                        if (!dstRT) return;
                        auto* dstFBO = dstRT->GetGfxFrameBufferObject().get();
                        if (!dstFBO) return;

                        // 每帧更新 PP_InColor 绑定（场景颜色来源变化时需要重新 Submit）
                        auto* srcRT = passCtx.Get(hSrc);
                        if (srcRT)
                        {
                            auto srcView = srcRT->GetGfxRenderTarget0();
                            if (srcView)
                            {
                                // FindByBinding 找已有的 descriptor，没有才 Add，避免每帧重复追加
                                auto* desc = ppRendererSet->FindByBinding(0);
                                if (!desc)
                                    desc = ppRendererSet->AddDescriptor("PP_InColor", 0);
                                desc->SetTextureSampler2D(srcView.get());
                            }
                        }
                        ppRendererSet->Submit();

                        auto* gfxApp          = cmdBuffer.GetApplication();
                        auto* pipelineMgr     = gfxApp->GetGraphicsPipelineManager();
                        auto  matDescSetLayout = mat->GetDescriptorSetLayout();
                        if (!matDescSetLayout) return;

                        array_list<gfx::GFXDescriptorSetLayout_sp> descLayouts;
                        descLayouts.push_back(matDescSetLayout);                    // set 0
                        descLayouts.push_back(perPass->GetDescriptorSetLayout());   // set 1
                        descLayouts.push_back(ppRendererLayout);                    // set 2

                        auto& gpuPrograms = program->GetGpuPrograms();

                        gfx::GFXGraphicsPipelineStateParams psoParams{};
                        psoParams.DepthTestEnable  = false;
                        psoParams.DepthWriteEnable = false;
                        psoParams.CullMode         = gfx::GFXCullMode::None;

                        auto gfxPipeline = pipelineMgr->GetGraphicsPipeline(
                            gpuPrograms, psoParams, descLayouts,
                            dstFBO->GetRenderTargetDesc(), {});
                        if (!gfxPipeline) return;

                        cmdBuffer.CmdSetViewport(0, 0, (float)dstFBO->GetWidth(), (float)dstFBO->GetHeight());
                        cmdBuffer.CmdBindGraphicsPipeline(gfxPipeline.get());

                        array_list<gfx::GFXDescriptorSet*> descSets;
                        descSets.push_back(mat->GetDescriptorSet().get());          // set 0
                        descSets.push_back(perPass->GetDescriptorSet().get());      // set 1
                        descSets.push_back(ppRendererSet);                          // set 2
                        cmdBuffer.CmdBindDescriptorSets(descSets, gfxPipeline.get());

                        // Full-screen triangle
                        cmdBuffer.CmdDraw(3);
                    });

                // 每次 pass 后交换，保证下一个 pass 的 Read 和 Write 始终是不同纹理
                std::swap(hSrc, hDst);
            }

            // 经过奇数个 pass 后，最终结果在 hPingPong（此时 hSrc == hPingPong）
            // 需要再做一次 Blit 把结果写回 hFinal
            if (hSrc != hFinal)
            {
                // hFinal 是 Write 资源，passCtx.Get 取不到；提前拿好 dstView
                auto* camFinalRT  = cam->GetRenderTexture().GetPtr();
                auto  capturedDstView = camFinalRT ? camFinalRT->GetGfxRenderTarget0() : nullptr;

                graph.AddPass("PostProcess_CopyToFinal")
                    .Read(hSrc)
                    .Write(hFinal, RGAttachmentDesc{
                        .colorLoadOp  = gfx::GFXRenderPassLoadOp::DontCare,
                        .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                    })
                    .WithPerPass(perPass)
                    .Execute([hSrc, capturedDstView](RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
                    {
                        auto* srcRT = passCtx.Get(hSrc);
                        if (!srcRT || !capturedDstView) return;

                        auto srcView = srcRT->GetGfxRenderTarget0();
                        if (!srcView) return;

                        cmdBuffer.CmdBlit(srcView.get(), capturedDstView.get());
                    });
            }
        }
    }

} // namespace pulsar
