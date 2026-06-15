#include "DefaultSceneCaptureRenderer.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Rendering/SceneView.h>
#include <Pulsar/Rendering/RenderScene.h>
#include <Pulsar/Rendering/LightProxy.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/TextureClasses.h>

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
        m_outlinePass.Initialize(&m_perPassResources);
        m_translucencyPass.Initialize(&m_perPassResources);
        m_gizmoOverlayPass.Initialize(&m_perPassResources);

        m_postProcessFeatures.push_back(std::make_unique<TonemapPass>());
        m_postProcessFeatures.push_back(std::make_unique<CustomPostProcessChain>());
        m_postProcessFeatures.push_back(std::make_unique<ColorGradingPass>());
        {
            auto bloom = std::make_unique<BloomPass>();
            bloom->Initialize();
            m_postProcessFeatures.push_back(std::move(bloom));
        }
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
            else if (auto* bloom = dynamic_cast<BloomPass*>(feature.get()))
                bloom->Destroy();
        }
        m_gizmoOverlayPass.Destroy();
        m_translucencyPass.Destroy();
        m_outlinePass.Destroy();
        m_opaquePass.Destroy();
        m_perPassResources.Destroy();
    }

    void DefaultSceneCaptureRenderer::Render(RenderGraph& graph, const RenderCaptureContext& ctx)
    {
        const SceneViewData* view = ctx.view;
        auto* scene = ctx.scene;

        if (!view || !scene)
            return;

        const RenderTargetSnapshot& camRenderTexture = view->RenderTarget;
        if (!camRenderTexture.IsValid())
            return;

        auto& perRenderObjectMgr = ctx.scene->GetPerRenderObjectData();
        perRenderObjectMgr.BeginFrame();

        auto* perPass = &m_perPassResources;

        PerPassCameraData camData{};
        camData.MatrixV     = view->ViewMatrix;
        camData.MatrixP     = view->ProjectionMatrix;
        camData.MatrixVP    = camData.MatrixP * camData.MatrixV;
        camData.InvMatrixV  = jmath::Inverse(camData.MatrixV);
        camData.InvMatrixP  = jmath::Inverse(camData.MatrixP);
        camData.InvMatrixVP = jmath::Inverse(camData.MatrixVP);
        camData.CamPosition = Vector4f(view->CameraPosition, 1.f);
        camData.CamNear     = view->Near;
        camData.CamFar      = view->Far;
        camData.Resolution  = view->Resolution;
        perPass->UpdateCamera(camData);

        {
            PerPassWorldData worldData{};
            worldData.TotalTime  = ctx.scene ? ctx.scene->GetTotalTime() : 0.f;
            worldData.DeltaTime  = ctx.scene ? ctx.scene->GetDeltaTime() : 0.f;

            if (ctx.scene)
            {
                const DirectionalLightProxy* brightest = nullptr;
                for (const auto& dir : ctx.scene->GetDirectionalLights())
                {
                    if (!brightest || dir->Intensity > brightest->Intensity)
                        brightest = dir.get();
                }
                if (brightest)
                {
                    worldData.WorldSpaceLightVector = -brightest->Vector;
                    auto& c = brightest->Color;
                    worldData.WorldSpaceLightColor  = {c.r, c.g, c.b, brightest->Intensity};
                }
            }
            worldData.SkyLightColor = {0, 0, 0, 0};
            worldData.LightParameterCount = ctx.scene ? static_cast<uint32_t>(ctx.scene->GetPointLights().size()) : 0;
            perPass->UpdateWorld(worldData);
        }

        {
            PerPassLightsBufferData lightsData{};
            if (ctx.scene)
            {
                auto& pointLights = ctx.scene->GetPointLights();
                int lightCount = std::min(static_cast<int>(pointLights.size()), 63);
                for (int i = 0; i < lightCount; ++i)
                {
                    lightsData.Lights[i] = pointLights[i]->Param;
                }
            }
            perPass->UpdateLights(lightsData);
        }

        RGTextureHandle hFinal = graph.ImportTexture("FinalOutput",
            camRenderTexture.Width, camRenderTexture.Height,
            camRenderTexture.Attachments, camRenderTexture.Framebuffer);

        uint32_t msaaSamples = std::max(1u, view->MSAASamples);

        RGTextureHandle hSceneColor = hFinal;
        gfx::GFXTexture2DView* resolveTargetView = nullptr;
        if (msaaSamples > 1)
        {
            RGTextureDesc msDesc{};
            msDesc.Width = camRenderTexture.Width;
            msDesc.Height = camRenderTexture.Height;
            msDesc.SampleCount = msaaSamples;
            for (auto& rt : camRenderTexture.Attachments)
            {
                bool isTransient = (rt->GetTargetType() == gfx::GFXTextureTargetType::ColorTarget);
                msDesc.TargetInfos.push_back({ rt->GetTargetType(), rt->GetFormat(), msaaSamples, isTransient });
            }
            hSceneColor = graph.CreateTransient("MSSceneColor", msDesc);
            resolveTargetView = camRenderTexture.GetRenderTarget0().get();
        }

        // OpaquePass (auto-resolve to final RT if MSAA is enabled)
        m_opaquePass.OnSetup(ctx);
        m_opaquePass.SetResolveTargetView(resolveTargetView);
        hSceneColor = m_opaquePass.AddToGraph(graph, hSceneColor, hSceneColor, ctx, perPass);

        // OutlinePass: draws vertex-expanded back-faces for materials with a VertexOutline pass
        m_outlinePass.OnSetup(ctx);
        hSceneColor = m_outlinePass.AddToGraph(graph, hSceneColor, hSceneColor, ctx, perPass);

        // ---- Translucency: copy opaque scene color for refraction/distortion sampling ----
        const RenderTargetSnapshot& camRT = camRenderTexture;
        auto hdrFormat = gfx::GFXTextureFormat::R16G16B16A16_SFloat;

        RGTextureDesc opaqueColorDesc{};
        opaqueColorDesc.Width  = camRT.Width;
        opaqueColorDesc.Height = camRT.Height;
        opaqueColorDesc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, hdrFormat });
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
                auto srcView = srcRT->GetRenderTarget0();
                auto dstView = dstRT->GetRenderTarget0();
                if (!srcView || !dstView) return;
                cmdBuffer.CmdBlit(srcView.get(), dstView.get());
            });

        // TranslucencyPass continues drawing onto the final target
        m_translucencyPass.OnSetup(ctx);
        m_translucencyPass.SetOpaqueColor(hOpaqueColor);
        hSceneColor = m_translucencyPass.AddToGraph(graph, hSceneColor, hSceneColor, ctx, perPass);

        // ---- Post-Process Features ----
        const VolumeStack& stack = view->PostProcessStack;

        RGTextureDesc pingPongDesc{};
        pingPongDesc.Width  = camRT.Width;
        pingPongDesc.Height = camRT.Height;
        pingPongDesc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, hdrFormat });

        RGTextureHandle hPingPongA = graph.CreateTransient("PostProcessPingPongA", pingPongDesc);
        RGTextureHandle hPingPongB = graph.CreateTransient("PostProcessPingPongB", pingPongDesc);

        RGTextureHandle hSrc = hFinal;
        RGTextureHandle hDst = hPingPongA;

        for (auto& feature : m_postProcessFeatures)
        {
            feature->OnSetup(ctx);
            feature->ReadSettings(stack);
            if (feature->IsEnabled())
            {
                hDst = feature->AddToGraph(graph, hSrc, hDst, ctx, perPass);
                std::swap(hSrc, hDst);
                if (hDst == hFinal)
                    hDst = (hSrc == hPingPongA) ? hPingPongB : hPingPongA;
            }
        }

        // Copy final result back to camera RT if needed
        if (hSrc != hFinal)
        {
            gfx::GFXTexture2DView_sp fallbackView = camRenderTexture.GetRenderTarget0();
            graph.AddPass("PostProcess_CopyToFinal")
                .Read(hSrc)
                .Write(hFinal)
                .NoRenderPass()
                .Execute([hSrc, hFinal, fallbackView](RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
                {
                    const auto* srcRT   = passCtx.Get(hSrc);
                    const auto* finalRT = passCtx.Get(hFinal);
                    gfx::GFXTexture2DView* finalView = nullptr;
                    if (finalRT)
                    {
                        finalView = finalRT->GetRenderTarget0().get();
                    }
                    else if (fallbackView)
                    {
                        finalView = fallbackView.get();
                    }
                    if (!srcRT || !finalView) return;

                    auto srcView = srcRT->GetRenderTarget0();
                    if (!srcView) return;

                    cmdBuffer.CmdBlit(srcView.get(), finalView);
                });
        }

        // Draw gizmos after all post-processing so they remain unaffected
        if (view->GizmoPassEnabled)
        {
            m_gizmoOverlayPass.OnSetup(ctx);
            m_gizmoOverlayPass.AddToGraph(graph, hFinal, hFinal, ctx, perPass);
        }

        perRenderObjectMgr.EndFrame();
    }

} // namespace pulsar
