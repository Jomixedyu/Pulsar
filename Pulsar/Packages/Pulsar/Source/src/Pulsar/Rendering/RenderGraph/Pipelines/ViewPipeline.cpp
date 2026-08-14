#include "ViewPipeline.h"
#include "SceneCaptureFrameData.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Rendering/SceneView.h>
#include <Pulsar/Rendering/RenderScene.h>
#include <Pulsar/Rendering/LightProxy.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/TextureClasses.h>
#include <cmath>

#include "../Modules/TonemapRenderModule.h"
#include "../Modules/DisplayEncodingRenderModule.h"
#include "../Modules/ColorGradingRenderModule.h"
#include "../Modules/CustomPostProcessRenderModule.h"
#include "../Modules/GizmoOverlayRenderModule.h"

namespace pulsar
{
    static bool IsRendererVisible(const rendering::RenderObject& renderer, const SceneViewData& view)
    {
        if (!renderer.HasBounds())
            return true;

        const auto bounds = renderer.GetBounds();
        if (bounds.Radius <= 0.f)
            return true;

        const Matrix4f viewProjection = view.ProjectionMatrix * view.ViewMatrix;
        const Vector4f clipCenter = viewProjection * Vector4f(bounds.Origin, 1.f);
        const float clipW = std::abs(clipCenter.w);
        if (clipW <= 0.0001f)
            return false;

        const float radiusX = std::abs(view.ProjectionMatrix[0][0]) * bounds.Radius;
        const float radiusY = std::abs(view.ProjectionMatrix[1][1]) * bounds.Radius;
        const float radiusZ = std::abs(view.ProjectionMatrix[2][2]) * bounds.Radius;
        return clipCenter.x >= -clipW - radiusX
            && clipCenter.x <= clipW + radiusX
            && clipCenter.y >= -clipW - radiusY
            && clipCenter.y <= clipW + radiusY
            && clipCenter.z >= -radiusZ
            && clipCenter.z <= clipW + radiusZ;
    }

    static SceneViewCullingFrameData CullRenderers(const RenderScene& scene, const SceneViewData& view)
    {
        auto renderers = std::make_shared<array_list<rendering::RenderObject_sp>>();
        renderers->reserve(scene.GetRenderObjects().size());
        for (const auto& renderer : scene.GetRenderObjects())
        {
            if (renderer && IsRendererVisible(*renderer, view))
                renderers->push_back(renderer);
        }
        return { std::move(renderers) };
    }

    ViewPipeline::ViewPipeline()
    {
        m_opaqueModule.Initialize();
        m_outlineModule.Initialize();
        m_translucencyModule.Initialize();
        m_gizmoOverlayModule.Initialize();

        m_postProcessRenderModules.push_back(std::make_unique<TonemapRenderModule>());
        m_postProcessRenderModules.push_back(std::make_unique<CustomPostProcessRenderModule>());
        m_postProcessRenderModules.push_back(std::make_unique<ColorGradingRenderModule>());
        m_postProcessRenderModules.push_back(std::make_unique<BloomRenderModule>());
        m_postProcessRenderModules.push_back(std::make_unique<DisplayEncodingRenderModule>());

        for (auto& module : m_postProcessRenderModules)
            module->Initialize();
    }

    ViewPipeline::~ViewPipeline()
    {
        for (auto& feature : m_featureProxies)
            feature->OnDestroyResource();

        for (auto& module : m_postProcessRenderModules)
            module->Destroy();
        m_gizmoOverlayModule.Destroy();
        m_translucencyModule.Destroy();
        m_outlineModule.Destroy();
        m_opaqueModule.Destroy();
    }

    void ViewPipeline::OnCreateResource(const ViewPipelineRenderData& data)
    {
        RebuildFeatureProxies(data);
    }

    void ViewPipeline::ApplyRenderData(const ViewPipelineRenderData& data)
    {
        RebuildFeatureProxies(data);
    }

    void ViewPipeline::RebuildFeatureProxies(const ViewPipelineRenderData& data)
    {
        for (auto& feature : m_featureProxies)
            feature->OnDestroyResource();
        m_featureProxies.clear();

        for (const auto& featureData : data.Features)
        {
            if (!featureData)
                continue;

            auto feature = featureData->CreateProxy();
            if (!feature)
                continue;

            feature->OnCreateResource();
            m_featureProxies.push_back(std::move(feature));
        }
    }

    void ViewPipeline::RecordFeatures(RenderGraph& graph, RenderFrameData& frameData)
    {
        for (const auto& feature : m_featureProxies)
        {
            feature->OnRecord(graph, frameData);
        }
    }
    void ViewPipeline::OnRecord(RenderGraph& graph, RenderFrameData& frameData)
    {
        auto* capture = frameData.Get<SceneCaptureFrameData>();
        if (!capture)
            return;

        const SceneViewData* view = capture->view;
        auto* scene = capture->scene;

        if (!view || !scene)
            return;

        const RenderTargetSnapshot& camRenderTexture = view->RenderTarget;
        if (!camRenderTexture.IsValid())
            return;

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

        frameData.Set(SceneCaptureGpuFrameData{
            capture->viewProxy ? capture->viewProxy->GetCameraBuffer() : nullptr,
            scene->GetWorldBuffer(),
            scene->GetLightsBuffer(),
            scene->GetPerRenderObjectData().GetBuffer(),
        });
        auto& sceneTarget = frameData.Set(SceneRenderTargetFrameData{ hSceneColor });
        frameData.Set(CullRenderers(*scene, *view));

        RecordFeatures(graph, frameData);

        // OpaqueRenderModule (auto-resolve to final RT if MSAA is enabled)
        m_opaqueModule.SetResolveTargetView(resolveTargetView);
        m_opaqueModule.OnRecord(graph, frameData);

        // OutlineRenderModule: draws vertex-expanded back-faces for materials with a VertexOutline pass
        m_outlineModule.OnRecord(graph, frameData);

        // ---- Translucency: copy opaque scene color for refraction/distortion sampling ----
        RGTextureDesc opaqueColorDesc{};
        opaqueColorDesc.Width  = camRenderTexture.Width;
        opaqueColorDesc.Height = camRenderTexture.Height;
        opaqueColorDesc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, hdrFormat });
        auto& opaqueColor = frameData.Set(SceneOpaqueColorFrameData{ graph.CreateTransient("OpaqueColorTexture", opaqueColorDesc) });


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

        // TranslucencyRenderModule continues drawing onto the final target
        m_translucencyModule.OnRecord(graph, frameData);

        // Preserve the old post-process input semantics: post-processing starts from the final camera RT.

        // ---- Post-Process Features ----
        RGTextureDesc pingPongDesc{};
        pingPongDesc.Width  = camRenderTexture.Width;
        pingPongDesc.Height = camRenderTexture.Height;
        pingPongDesc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, hdrFormat });

        auto hPostProcessA = graph.CreateTransient("PostProcessPingPongA", pingPongDesc);
        auto hPostProcessB = graph.CreateTransient("PostProcessPingPongB", pingPongDesc);
        auto& postProcess = frameData.Set(ScenePostProcessFrameData{
            .FinalTarget = hFinal,
            .ActiveColor = hFinal,
            .PingPongA = hPostProcessA,
            .PingPongB = hPostProcessB,
        });
        for (auto& module : m_postProcessRenderModules)
        {
            module->OnRecord(graph, frameData);
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
            m_gizmoOverlayModule.OnRecord(graph, frameData);
        }

    }

} // namespace pulsar
