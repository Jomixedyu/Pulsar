#include "DefaultSceneCaptureRenderer.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Rendering/SceneView.h>
#include <Pulsar/Rendering/RenderScene.h>
#include <Pulsar/Rendering/LightProxy.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/TextureClasses.h>

#include "Passes/TonemapRenderFeature.h"
#include "Passes/DisplayEncodingRenderFeature.h"
#include "Passes/ColorGradingRenderFeature.h"
#include "Passes/CustomPostProcessRenderFeature.h"
#include "Passes/GizmoOverlayRenderFeature.h"

namespace pulsar
{
    DefaultSceneCaptureRenderer::DefaultSceneCaptureRenderer()
    {
        m_opaqueFeature.Initialize();
        m_outlineFeature.Initialize();
        m_translucencyFeature.Initialize();
        m_gizmoOverlayFeature.Initialize();

        m_postProcessRenderFeatures.push_back(std::make_unique<TonemapRenderFeature>());
        m_postProcessRenderFeatures.push_back(std::make_unique<CustomPostProcessRenderFeature>());
        m_postProcessRenderFeatures.push_back(std::make_unique<ColorGradingRenderFeature>());
        m_postProcessRenderFeatures.push_back(std::make_unique<BloomRenderFeature>());
        m_postProcessRenderFeatures.push_back(std::make_unique<DisplayEncodingRenderFeature>());

        for (auto& feature : m_postProcessRenderFeatures)
            feature->Initialize();
    }

    DefaultSceneCaptureRenderer::~DefaultSceneCaptureRenderer()
    {
        for (auto& feature : m_postProcessRenderFeatures)
            feature->Destroy();
        m_gizmoOverlayFeature.Destroy();
        m_translucencyFeature.Destroy();
        m_outlineFeature.Destroy();
        m_opaqueFeature.Destroy();
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
        if (ctx.viewProxy)
            ctx.viewProxy->UploadCamera(camData);

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
            if (ctx.scene)
                ctx.scene->UploadWorld(worldData);
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
            if (ctx.scene)
                ctx.scene->UploadLights(lightsData);
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

        auto hdrFormat = gfx::GFXTextureFormat::R16G16B16A16_SFloat;

        RenderFrameData frameData;
        frameData.Set(ctx);
        frameData.Set(ViewFrameData{ view, ctx.viewProxy, ctx.frameIndex });
        frameData.Set(SceneFrameData{ scene });
        frameData.Set(GpuFrameData{
            ctx.viewProxy ? ctx.viewProxy->GetCameraBuffer() : nullptr,
            scene->GetWorldBuffer(),
            scene->GetLightsBuffer(),
            scene->GetPerRenderObjectData().GetBuffer(),
        });
        auto& sceneTarget = frameData.Set(SceneTargetFrameData{ hSceneColor });

        // OpaqueRenderFeature (auto-resolve to final RT if MSAA is enabled)
        m_opaqueFeature.SetResolveTargetView(resolveTargetView);
        m_opaqueFeature.OnRecord(graph, frameData);

        // OutlineRenderFeature: draws vertex-expanded back-faces for materials with a VertexOutline pass
        m_outlineFeature.OnRecord(graph, frameData);

        // ---- Translucency: copy opaque scene color for refraction/distortion sampling ----
        RGTextureDesc opaqueColorDesc{};
        opaqueColorDesc.Width  = camRenderTexture.Width;
        opaqueColorDesc.Height = camRenderTexture.Height;
        opaqueColorDesc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, hdrFormat });
        auto& opaqueColor = frameData.Set(OpaqueColorFrameData{ graph.CreateTransient("OpaqueColorTexture", opaqueColorDesc) });


        graph.AddPass("CopyOpaqueColor")
            .Read(sceneTarget.Target)
            .Write(opaqueColor.Color)
            .NoRenderPass()
            .Execute([hSceneColor = sceneTarget.Target, hOpaqueColor = opaqueColor.Color](RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
            {
                auto* srcRT = passCtx.Get(hSceneColor);
                auto* dstRT = passCtx.Get(hOpaqueColor);
                if (!srcRT || !dstRT) return;
                auto srcView = srcRT->GetRenderTarget0();
                auto dstView = dstRT->GetRenderTarget0();
                if (!srcView || !dstView) return;
                cmdBuffer.CmdBlit(srcView.get(), dstView.get());
            });

        // TranslucencyRenderFeature continues drawing onto the final target
        m_translucencyFeature.OnRecord(graph, frameData);

        // Preserve the old post-process input semantics: post-processing starts from the final camera RT.

        // ---- Post-Process Features ----
        RGTextureDesc pingPongDesc{};
        pingPongDesc.Width  = camRenderTexture.Width;
        pingPongDesc.Height = camRenderTexture.Height;
        pingPongDesc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, hdrFormat });

        auto hPostProcessA = graph.CreateTransient("PostProcessPingPongA", pingPongDesc);
        auto hPostProcessB = graph.CreateTransient("PostProcessPingPongB", pingPongDesc);
        auto& postProcess = frameData.Set(PostProcessFrameData{
            .FinalTarget = hFinal,
            .ActiveColor = hFinal,
            .PingPongA = hPostProcessA,
            .PingPongB = hPostProcessB,
        });
        for (auto& feature : m_postProcessRenderFeatures)
        {
            feature->OnRecord(graph, frameData);
        }

        // Copy final result back to camera RT if needed
        if (postProcess.ActiveColor != postProcess.FinalTarget)
        {
            gfx::GFXTexture2DView_sp fallbackView = camRenderTexture.GetRenderTarget0();
            graph.AddPass("PostProcess_CopyToFinal")
                .Read(postProcess.ActiveColor)
                .Write(postProcess.FinalTarget)
                .NoRenderPass()
                .Execute([hSrc = postProcess.ActiveColor, hFinal = postProcess.FinalTarget, fallbackView](RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
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
            m_gizmoOverlayFeature.OnRecord(graph, frameData);
        }

        perRenderObjectMgr.EndFrame();
    }

} // namespace pulsar
