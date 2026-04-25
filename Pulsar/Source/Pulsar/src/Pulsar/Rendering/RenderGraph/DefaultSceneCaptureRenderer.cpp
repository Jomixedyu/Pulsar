#include "DefaultSceneCaptureRenderer.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Node.h>
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Components/SceneCaptureComponent.h>
#include <Pulsar/Components/VolumeComponent.h>

#include <Pulsar/World.h>
#include <Pulsar/Scene.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Rendering/ShaderPass.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/AssetManager.h>
#include <Pulsar/Application.h>
#include <Pulsar/Subsystems/PostProcessSubsystem.h>
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

    void DefaultSceneCaptureRenderer::EnsureTonemapMaterial()
    {
        if (m_tonemapMaterial)
            return;

        auto shader = AssetManager::Get()->LoadAsset<Shader>("Engine/Shaders/Tonemap");
        if (!shader)
        {
            Logger::Log("DefaultSceneCaptureRenderer: failed to load Tonemap shader", LogLevel::Warning);
            return;
        }

        m_tonemapMaterial = Material::StaticCreate(shader);
        m_tonemapMaterial->CreateGPUResource();
    }

    void DefaultSceneCaptureRenderer::EnsureGammaMaterial()
    {
        if (m_gammaMaterial)
            return;

        auto shader = AssetManager::Get()->LoadAsset<Shader>("Engine/Shaders/Gamma");
        if (!shader)
        {
            Logger::Log("DefaultSceneCaptureRenderer: failed to load Gamma shader", LogLevel::Warning);
            return;
        }

        m_gammaMaterial = Material::StaticCreate(shader);
        m_gammaMaterial->CreateGPUResource();
    }

    void DefaultSceneCaptureRenderer::Render(RenderGraph& graph, const RenderCaptureContext& ctx)
    {
        auto* cam   = dynamic_cast<CameraComponent*>(ctx.capture);
        auto* world = ctx.world;

        if (!cam || !world)
            return;

        auto* camRenderTexture = cam->GetRenderTexture().GetPtr();
        if (!camRenderTexture)
            return;

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
            PerPassWorldData worldData{};
            worldData.TotalTime  = world->GetTotalTime();
            worldData.DeltaTime  = world->GetTicker().deltatime;

            if (auto* env = world->GetFocusScene()->GetRuntimeEnvironment())
            {
                if (const auto* dirLight = env->GetDirectionalLight())
                {
                    worldData.WorldSpaceLightVector = -dirLight->Vector;
                    auto& c = dirLight->Color;
                    worldData.WorldSpaceLightColor  = {c.r, c.g, c.b, dirLight->Intensity};
                }
                {
                    auto skyLight = env->GetSkyLight();
                    worldData.SkyLightColor = {skyLight.Color.r, skyLight.Color.g, skyLight.Color.b, skyLight.Intensity};
                }
            }
            worldData.LightParameterCount = static_cast<uint32_t>(world->GetLightManager()->GetLightCount());
            perPass->UpdateWorld(worldData);
        }

        {
            PerPassLightsBufferData lightsData{};
            auto* lightMgr = ctx.world->GetLightManager();
            int lightCount = std::min(lightMgr->GetLightCount(), 63);
            for (int i = 0; i < lightCount; ++i)
            {
                auto& src = lightMgr->GetLightParameter(i);
                lightsData.Lights[i] = src;
            }
            perPass->UpdateLights(lightsData);
        }

        perPass->Submit();

        RGTextureHandle hFinal = graph.ImportTexture("FinalOutput", camRenderTexture);

        // PreparedBatch: batch + its ready binding (nullptr if shader not yet compiled)
        struct PreparedBatch
        {
            rendering::MeshBatch       batch;
            const MaterialPassBinding* binding = nullptr;
        };
        auto preparedOpaque      = std::make_shared<array_list<PreparedBatch>>();
        auto preparedAlphaTest   = std::make_shared<array_list<PreparedBatch>>();
        auto preparedTransparent = std::make_shared<array_list<PreparedBatch>>();

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
            .Prepare([cam, world, preparedOpaque, preparedAlphaTest, preparedTransparent](RGPassContext&)
            {
                // Collect batches and prepare material GPU resources.
                // This runs after Compile() but before BeginRenderPass — safe for resource creation.
                const Vector3f camPos     = cam->GetNode()->GetTransform()->GetWorldPosition();
                const Vector3f camForward = cam->GetNode()->GetTransform()->GetForward();

                for (const rendering::RenderObject_sp& ro : world->GetRenderObjects())
                {
                    const float depth = jmath::Dot(camForward, ro->GetWorldPosition() - camPos);
                    for (auto batch : ro->GetMeshBatches())
                    {
                        batch.Depth = depth;
                        const MaterialPassBinding* binding = batch.Material
                            ? batch.Material->PrepareForRendering("Forward", batch.Interface)
                            : nullptr;

                        PreparedBatch pb{ std::move(batch), binding };
                        switch (pb.batch.Queue)
                        {
                        case ShaderPassRenderQueueType::AlphaTest:
                            preparedAlphaTest->push_back(std::move(pb));
                            break;
                        case ShaderPassRenderQueueType::Transparency:
                            preparedTransparent->push_back(std::move(pb));
                            break;
                        default:
                            preparedOpaque->push_back(std::move(pb));
                            break;
                        }
                    }
                }

                auto sortAsc  = [](const PreparedBatch& a, const PreparedBatch& b)
                {
                    if (a.batch.Priority != b.batch.Priority) return a.batch.Priority < b.batch.Priority;
                    return a.batch.Depth < b.batch.Depth;
                };
                auto sortDesc = [](const PreparedBatch& a, const PreparedBatch& b)
                {
                    if (a.batch.Priority != b.batch.Priority) return a.batch.Priority < b.batch.Priority;
                    return a.batch.Depth > b.batch.Depth;
                };
                std::sort(preparedOpaque->begin(),      preparedOpaque->end(),      sortAsc);
                std::sort(preparedAlphaTest->begin(),   preparedAlphaTest->end(),   sortAsc);
                std::sort(preparedTransparent->begin(), preparedTransparent->end(), sortDesc);
            })
            .Execute([cam, perPass, preparedOpaque, preparedAlphaTest, preparedTransparent]
                     (RGPassContext& ctx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                auto* targetFBO = cam->GetRenderTexture()->GetGfxFrameBufferObject().get();
                if (!targetFBO) return;

                auto* pipelineMgr = cmdBuffer.GetApplication()->GetGraphicsPipelineManager();
                cmdBuffer.CmdSetViewport(0, 0, (float)targetFBO->GetWidth(), (float)targetFBO->GetHeight());

                auto DrawBatchList = [&](const array_list<PreparedBatch>& entries)
                {
                    for (const auto& pb : entries)
                    {
                        if (!pb.binding) continue; // shader not ready yet

                        auto program = pb.binding->GetCurrentProgram();
                        if (!program) continue;

                        auto shader = pb.batch.Material->GetShader();
                        if (!shader || !shader->GetConfig()) continue;

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
                                psoParams.DepthTestEnable   = !pb.batch.IsDepthTestDisabled;
                                psoParams.StencilTestEnable = gp->Stencil_Enabled;
                            }
                        }
                        // IsDepthTestDisabled 时强制关闭 depth write 和 test
                        if (pb.batch.IsDepthTestDisabled)
                        {
                            psoParams.DepthTestEnable  = false;
                            psoParams.DepthWriteEnable = false;
                        }

                        array_list<gfx::GFXDescriptorSetLayout_sp> descLayouts;
                        descLayouts.push_back(pb.binding->m_descriptorSetLayout);
                        descLayouts.push_back(perPass->GetDescriptorSetLayout());
                        descLayouts.push_back(pb.batch.DescriptorSetLayout);

                        auto gfxPipeline = pipelineMgr->GetGraphicsPipeline(
                            gpuPrograms, psoParams, descLayouts,
                            targetFBO->GetRenderTargetDesc(), pb.batch.State);

                        cmdBuffer.CmdBindGraphicsPipeline(gfxPipeline.get());
                        cmdBuffer.CmdSetCullMode(pb.batch.GetCullMode());

                        for (const auto& element : pb.batch.Elements)
                        {
                            array_list<gfx::GFXDescriptorSet*> descSets;
                            descSets.push_back(pb.binding->m_descriptorSet.get());
                            descSets.push_back(perPass->GetDescriptorSet().get());
                            descSets.push_back(element.ModelDescriptor.get());
                            cmdBuffer.CmdBindDescriptorSets(descSets, gfxPipeline.get());

                            cmdBuffer.CmdBindVertexBuffers({element.Vertex.get()});
                            if (pb.batch.IsUsedIndices)
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

                DrawBatchList(*preparedOpaque);
                DrawBatchList(*preparedAlphaTest);
                DrawBatchList(*preparedTransparent);
            });

        // ---- Post-Process Passes ----
        EnsureTonemapMaterial();
        EnsureGammaMaterial();

        BlendedPostProcessSettings ppSettings{};
        if (auto* ppSub = world->GetSubsystem<PostProcessSubsystem>())
        {
            auto camPos = cam->GetNode()->GetTransform()->GetWorldPosition();
            ppSettings = ppSub->QuerySettings(camPos);
        }

        array_list<RCPtr<Material>> ppMaterials;
        if (auto* ppSub = world->GetSubsystem<PostProcessSubsystem>())
        {
            auto camPos = cam->GetNode()->GetTransform()->GetWorldPosition();
            ppMaterials = ppSub->QueryPostProcessMaterials(camPos);
        }

        bool doPostProcess = ppSettings.enabled || !ppMaterials.empty();

        if (doPostProcess)
        {
            auto* camRT = cam->GetRenderTexture().GetPtr();
            RGTextureDesc pingPongDesc{};
            pingPongDesc.Width  = camRT->GetWidth();
            pingPongDesc.Height = camRT->GetHeight();
            auto format = camRT->GetRenderTargets()[0]->GetFormat();
            pingPongDesc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, format });

            RGTextureHandle hPingPong = graph.CreateTransient("PostProcessPingPong", pingPongDesc);
            RGTextureHandle hSrc = hFinal;
            RGTextureHandle hDst = hPingPong;

            if (!m_ppRendererLayout)
            {
                auto* gfxApp = Application::GetGfxApp();
                gfx::GFXDescriptorSetLayoutDesc desc{
                    gfx::GFXDescriptorType::CombinedImageSampler,
                    gfx::GFXGpuProgramStageFlags::Fragment,
                    0, 1
                };
                m_ppRendererLayout = gfxApp->CreateDescriptorSetLayout(&desc, 1);
            }

            auto  ppRendererLayout = m_ppRendererLayout;

            auto ExecutePPMaterial = [hFinal, cam, perPass, ppRendererLayout]
                (Material* mat, RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer,
                 RGTextureHandle srcHandle, RGTextureHandle dstHandle,
                 gfx::GFXDescriptorSet_sp& ppSet)
            {
                if (!mat) return;
                auto shader = mat->GetShader();
                if (!shader || !shader->GetConfig()) return;

                const auto* ppPassBinding = mat->GetPassBinding("PostProcess", "RENDERER_IMAGEPROCESS")
                                                .m_gpuResourcesInitialized
                                            ? &mat->GetPassBinding("PostProcess", "RENDERER_IMAGEPROCESS")
                                            : nullptr;
                if (!ppPassBinding) return;

                auto program = ppPassBinding->GetCurrentProgram();
                if (!program) return;

                auto* gfxApp = cmdBuffer.GetApplication();
                if (!ppSet)
                    ppSet = gfxApp->GetDescriptorManager()->GetDescriptorSet(ppRendererLayout);
                if (!ppSet) return;
                auto* ppRendererSet = ppSet.get();

                RenderTexture* dstRT = passCtx.Get(dstHandle);
                if (!dstRT && dstHandle == hFinal)
                    dstRT = cam->GetRenderTexture().GetPtr();
                if (!dstRT) return;
                auto* dstFBO = dstRT->GetGfxFrameBufferObject().get();
                if (!dstFBO) return;

                RenderTexture* srcRT = passCtx.Get(srcHandle);
                if (!srcRT && srcHandle == hFinal)
                    srcRT = cam->GetRenderTexture().GetPtr();
                if (srcRT)
                {
                    auto srcView = srcRT->GetGfxRenderTarget0();
                    if (srcView)
                    {
                        auto* desc = ppRendererSet->FindByBinding(0);
                        if (!desc)
                            desc = ppRendererSet->AddDescriptor("PP_InColor", 0);
                        desc->SetTextureSampler2D(srcView.get());
                    }
                }
                ppRendererSet->Submit();

                auto* pipelineMgr = gfxApp->GetGraphicsPipelineManager();
                array_list<gfx::GFXDescriptorSetLayout_sp> descLayouts;
                descLayouts.push_back(ppPassBinding->m_descriptorSetLayout);
                descLayouts.push_back(perPass->GetDescriptorSetLayout());
                descLayouts.push_back(ppRendererLayout);

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
                descSets.push_back(ppPassBinding->m_descriptorSet.get());
                descSets.push_back(perPass->GetDescriptorSet().get());
                descSets.push_back(ppRendererSet);
                cmdBuffer.CmdBindDescriptorSets(descSets, gfxPipeline.get());

                cmdBuffer.CmdDraw(3);
            };

            // 1. Tonemap pass (HDR -> LDR linear)
            if (ppSettings.enabled && ppSettings.hasTonemapping)
            {
                m_tonemapMaterial->SetIntScalar("_TonemappingMode", ppSettings.tonemappingMode);
                m_tonemapMaterial->SetIntScalar("_Enabled", 1);
                m_tonemapMaterial->SubmitParameters();

                auto curSrc = hSrc;
                auto curDst = hDst;
                graph.AddPass("PostProcess_Tonemap")
                    .Read(curSrc)
                    .Write(curDst, RGAttachmentDesc{
                        .colorLoadOp  = gfx::GFXRenderPassLoadOp::DontCare,
                        .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                    })
                    .WithPerPass(perPass)
                    .Prepare([this](RGPassContext&)
                    {
                        if (m_tonemapMaterial)
                            m_tonemapMaterial->PrepareForRendering("PostProcess", "RENDERER_IMAGEPROCESS");
                    })
                    .Execute([this, ExecutePPMaterial, curSrc, curDst]
                             (RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
                    {
                        ExecutePPMaterial(m_tonemapMaterial.GetPtr(), passCtx, cmdBuffer, curSrc, curDst, m_ppTonemapSet);
                    });

                std::swap(hSrc, hDst);
            }

            // 2. VolumeProfile custom post-process materials (LDR linear space)
            for (int i = 0; i < static_cast<int>(ppMaterials.size()); ++i)
            {
                auto* mat = ppMaterials[i].GetPtr();
                std::string passName = StringUtil::Concat("PostProcess_PPCompMat_", std::to_string(i));
                auto curSrc = hSrc;
                auto curDst = hDst;

                graph.AddPass(passName)
                    .Read(curSrc)
                    .Write(curDst, RGAttachmentDesc{
                        .colorLoadOp  = gfx::GFXRenderPassLoadOp::DontCare,
                        .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                    })
                    .WithPerPass(perPass)
                    .Prepare([mat](RGPassContext&)
                    {
                        if (mat)
                            mat->PrepareForRendering("PostProcess", "RENDERER_IMAGEPROCESS");
                    })
                    .Execute([this, ExecutePPMaterial, mat, curSrc, curDst]
                             (RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
                    {
                        ExecutePPMaterial(mat, passCtx, cmdBuffer, curSrc, curDst, m_ppCustomSet);
                    });

                std::swap(hSrc, hDst);
            }

            // 3. Gamma correction pass (last step before output)
            if (ppSettings.applyGamma)
            {
                m_gammaMaterial->SetFloat("_Gamma", ppSettings.gamma);
                m_gammaMaterial->SetIntScalar("_Enabled", 1);
                m_gammaMaterial->SubmitParameters();

                auto curSrc = hSrc;
                auto curDst = hDst;
                graph.AddPass("PostProcess_Gamma")
                    .Read(curSrc)
                    .Write(curDst, RGAttachmentDesc{
                        .colorLoadOp  = gfx::GFXRenderPassLoadOp::DontCare,
                        .colorStoreOp = gfx::GFXRenderPassStoreOp::Store,
                    })
                    .WithPerPass(perPass)
                    .Prepare([this](RGPassContext&)
                    {
                        if (m_gammaMaterial)
                            m_gammaMaterial->PrepareForRendering("PostProcess", "RENDERER_IMAGEPROCESS");
                    })
                    .Execute([this, ExecutePPMaterial, curSrc, curDst]
                             (RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
                    {
                        ExecutePPMaterial(m_gammaMaterial.GetPtr(), passCtx, cmdBuffer, curSrc, curDst, m_ppGammaSet);
                    });

                std::swap(hSrc, hDst);
            }

            // 4. Copy final result back to camera RT if needed
            if (hSrc != hFinal)
            {
                graph.AddPass("PostProcess_CopyToFinal")
                    .Read(hSrc)
                    .Write(hFinal)
                    .NoRenderPass()
                    .Execute([hSrc, hFinal, cam](RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
                    {
                        auto* srcRT   = passCtx.Get(hSrc);
                        auto* finalRT = passCtx.Get(hFinal);
                        if (!finalRT)
                            finalRT = cam->GetRenderTexture().GetPtr();
                        if (!srcRT || !finalRT) return;

                        auto srcView   = srcRT->GetGfxRenderTarget0();
                        auto finalView = finalRT->GetGfxRenderTarget0();
                        if (!srcView || !finalView) return;

                        cmdBuffer.CmdBlit(srcView.get(), finalView.get());
                    });
            }
        }
    }

} // namespace pulsar
