#include "DefaultSceneCaptureRenderer.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Node.h>
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Components/SceneCaptureComponent.h>
#include <Pulsar/Components/VolumeComponent.h>
#include <Pulsar/World.h>
#include <Pulsar/Scene.h>
#include <Pulsar/Subsystems/PostProcessSubsystem.h>
#include <gfx/GFXCommandBuffer.h>

#include "Passes/TonemapPass.h"
#include "Passes/GammaCorrectionPass.h"
#include "Passes/ColorGradingPass.h"
#include "Passes/CustomPostProcessChain.h"

namespace pulsar
{
    DefaultSceneCaptureRenderer::DefaultSceneCaptureRenderer()
    {
        m_perPassResources.Initialize();

        m_features.push_back(std::make_unique<TonemapPass>());
        m_features.push_back(std::make_unique<CustomPostProcessChain>());
        m_features.push_back(std::make_unique<ColorGradingPass>());
        m_features.push_back(std::make_unique<GammaCorrectionPass>());
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

        // BasePass
        hFinal = m_basePass.AddToGraph(graph, hFinal, cam, world, perPass);

        // ---- Post-Process Features ----
        VolumeStack stack;
        if (auto* ppSub = world->GetSubsystem<PostProcessSubsystem>())
        {
            auto camPos = cam->GetNode()->GetTransform()->GetWorldPosition();
            stack = ppSub->QuerySettings(camPos);
        }

        auto* camRT = cam->GetRenderTexture().GetPtr();
        RGTextureDesc pingPongDesc{};
        pingPongDesc.Width  = camRT->GetWidth();
        pingPongDesc.Height = camRT->GetHeight();
        auto format = camRT->GetRenderTargets()[0]->GetFormat();
        pingPongDesc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, format });

        RGTextureHandle hPingPong = graph.CreateTransient("PostProcessPingPong", pingPongDesc);
        RGTextureHandle hSrc = hFinal;
        RGTextureHandle hDst = hPingPong;

        for (auto& feature : m_features)
        {
            feature->OnSetup(ctx);
            feature->ReadSettings(stack);
            if (feature->IsEnabled())
            {
                hDst = feature->AddToGraph(graph, hSrc, hDst, cam, perPass);
                std::swap(hSrc, hDst);
            }
        }

        // Copy final result back to camera RT if needed
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

} // namespace pulsar
