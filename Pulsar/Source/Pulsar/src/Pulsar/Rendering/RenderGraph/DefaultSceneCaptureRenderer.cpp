#include "DefaultSceneCaptureRenderer.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Node.h>
#include <Pulsar/Components/CameraComponent.h>
#include <Pulsar/Components/SceneCaptureComponent.h>
#include <Pulsar/Components/SceneCapture2DComponent.h>
#include <Pulsar/Components/VolumeComponent.h>
#include <Pulsar/World.h>
#include <Pulsar/Scene.h>
#include <Pulsar/Subsystems/PostProcessSubsystem.h>
#include <gfx/GFXCommandBuffer.h>

#include "Passes/TonemapPass.h"
#include "Passes/DisplayEncodingPass.h"
#include "Passes/ColorGradingPass.h"
#include "Passes/CustomPostProcessChain.h"
#include "Passes/GizmoOverlayPass.h"

namespace pulsar
{
    DefaultSceneCaptureRenderer::DefaultSceneCaptureRenderer()
    {
        m_perPassResources.Initialize();
        m_opaquePass.Initialize(&m_perPassResources);
        m_translucencyPass.Initialize(&m_perPassResources);
        m_gizmoOverlayPass.Initialize(&m_perPassResources);

        m_postProcessFeatures.push_back(std::make_unique<TonemapPass>());
        m_postProcessFeatures.push_back(std::make_unique<CustomPostProcessChain>());
        m_postProcessFeatures.push_back(std::make_unique<ColorGradingPass>());
        m_postProcessFeatures.push_back(std::make_unique<DisplayEncodingPass>());

        for (auto& feature : m_postProcessFeatures)
        {
            if (auto* pp = dynamic_cast<PostProcessPass*>(feature.get()))
                pp->Initialize(&m_perPassResources);
        }
    }

    DefaultSceneCaptureRenderer::~DefaultSceneCaptureRenderer()
    {
        for (auto& feature : m_postProcessFeatures)
        {
            if (auto* pp = dynamic_cast<PostProcessPass*>(feature.get()))
                pp->Destroy();
        }
        m_gizmoOverlayPass.Destroy();
        m_translucencyPass.Destroy();
        m_opaquePass.Destroy();
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

        RGTextureHandle hFinal = graph.ImportTexture("FinalOutput", camRenderTexture);

        uint32_t msaaSamples = 1;
        if (auto* capture2D = dynamic_cast<SceneCapture2DComponent*>(ctx.capture))
        {
            msaaSamples = std::max(1u, capture2D->GetMSAASamples());
        }

        RGTextureHandle hSceneColor = hFinal;
        gfx::GFXTexture2DView* resolveTargetView = nullptr;
        if (msaaSamples > 1)
        {
            RGTextureDesc msDesc{};
            msDesc.Width = camRenderTexture->GetWidth();
            msDesc.Height = camRenderTexture->GetHeight();
            msDesc.SampleCount = msaaSamples;
            for (auto& rt : camRenderTexture->GetRenderTargets())
            {
                bool isTransient = (rt->GetTargetType() == gfx::GFXTextureTargetType::ColorTarget);
                msDesc.TargetInfos.push_back({ rt->GetTargetType(), rt->GetFormat(), msaaSamples, isTransient });
            }
            hSceneColor = graph.CreateTransient("MSSceneColor", msDesc);
            resolveTargetView = camRenderTexture->GetGfxRenderTarget0().get();
        }

        // OpaquePass (auto-resolve to final RT if MSAA is enabled)
        hSceneColor = m_opaquePass.AddToGraph(graph, hSceneColor, cam, world, perPass, resolveTargetView);

        // ---- Translucency: copy opaque scene color for refraction/distortion sampling ----
        auto* camRT = cam->GetRenderTexture().GetPtr();
        auto format = camRT->GetRenderTargets()[0]->GetFormat();

        RGTextureDesc opaqueColorDesc{};
        opaqueColorDesc.Width  = camRT->GetWidth();
        opaqueColorDesc.Height = camRT->GetHeight();
        opaqueColorDesc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, format });
        auto hOpaqueColor = graph.CreateTransient("OpaqueColorTexture", opaqueColorDesc);

        graph.AddPass("CopyOpaqueColor")
            .Read(hSceneColor)
            .Write(hOpaqueColor)
            .NoRenderPass()
            .Execute([hSceneColor, hOpaqueColor](RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                auto* srcRT = passCtx.Get(hSceneColor);
                auto* dstRT = passCtx.Get(hOpaqueColor);
                if (!srcRT || !dstRT) return;
                auto srcView = srcRT->GetGfxRenderTarget0();
                auto dstView = dstRT->GetGfxRenderTarget0();
                if (!srcView || !dstView) return;
                cmdBuffer.CmdBlit(srcView.get(), dstView.get());
            });

        // TranslucencyPass continues drawing onto the final target
        hSceneColor = m_translucencyPass.AddToGraph(graph, hSceneColor, cam, world, perPass, hOpaqueColor);

        // ---- Post-Process Features ----
        VolumeStack stack;
        if (auto* ppSub = world->GetSubsystem<PostProcessSubsystem>())
        {
            auto camPos = cam->GetNode()->GetTransform()->GetWorldPosition();
            stack = ppSub->QuerySettings(camPos);
        }

        RGTextureDesc pingPongDesc{};
        pingPongDesc.Width  = camRT->GetWidth();
        pingPongDesc.Height = camRT->GetHeight();
        pingPongDesc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, format });

        RGTextureHandle hPingPong = graph.CreateTransient("PostProcessPingPong", pingPongDesc);
        RGTextureHandle hSrc = hFinal;
        RGTextureHandle hDst = hPingPong;

        for (auto& feature : m_postProcessFeatures)
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

        // Draw gizmos after all post-processing so they remain unaffected
        m_gizmoOverlayPass.OnSetup(ctx);
        m_gizmoOverlayPass.AddToGraph(graph, hFinal, hFinal, cam, perPass);
    }

} // namespace pulsar
