#include "ViewPipeline.h"
#include "SceneCaptureFrameData.h"
#include <Pulsar/Rendering/SceneView.h>
#include <Pulsar/Rendering/RenderScene.h>
#include <Pulsar/Rendering/LightProxy.h>
#include <gfx/TextureClasses.h>
#include <gfx/GFXCommandBuffer.h>
#include <cmath>

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


    ViewPipeline::ViewPipeline() = default;

    ViewPipeline::~ViewPipeline()
    {
        for (auto& feature : m_features)
            feature->OnDestroyResource();
    }

    void ViewPipeline::OnCreateResource(const ViewPipelineRenderData& data)
    {
        RebuildFeatures(data);
    }

    void ViewPipeline::ApplyRenderData(const ViewPipelineRenderData& data)
    {
        RebuildFeatures(data);
    }

    bool ViewPipeline::HasSameFeatures(const ViewPipelineRenderData& data) const
    {
        return m_featureTypes == data.FeatureTypes;
    }

    void ViewPipeline::RebuildFeatures(const ViewPipelineRenderData& data)
    {
        for (auto& feature : m_features)
            feature->OnDestroyResource();
        m_features.clear();
        m_featureTypes.clear();

        for (const auto& featureFactory : data.Features)
        {
            if (!featureFactory)
                continue;

            auto feature = featureFactory();
            if (!feature)
                continue;

            feature->OnCreateResource();
            m_features.push_back(std::move(feature));

        }

        m_featureTypes = data.FeatureTypes;
    }

    void ViewPipeline::RecordFeatures(RenderGraph& graph, RenderFrameData& frameData)
    {
        for (const auto& feature : m_features)
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

        frameData.Set(SceneCaptureGpuFrameData{
            capture->viewProxy ? capture->viewProxy->GetCameraBuffer() : nullptr,
            scene->GetWorldBuffer(),
            scene->GetLightsBuffer(),
            scene->GetPerRenderObjectData().GetBuffer(),
        });
        frameData.Set(SceneRenderTargetFrameData{ hSceneColor });
        frameData.Set(SceneResolveTargetFrameData{ resolveTargetView });
        frameData.Set(CullRenderers(*scene, *view));

        RGTextureDesc pingPongDesc{};
        pingPongDesc.Width = camRenderTexture.Width;
        pingPongDesc.Height = camRenderTexture.Height;
        pingPongDesc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, gfx::GFXTextureFormat::R16G16B16A16_SFloat });

        auto hPostProcessA = graph.CreateTransient("PostProcessPingPongA", pingPongDesc);
        auto hPostProcessB = graph.CreateTransient("PostProcessPingPongB", pingPongDesc);
        auto& postProcess = frameData.Set(ScenePostProcessFrameData{
            .FinalTarget = hFinal,
            .ActiveColor = hFinal,
            .PingPongA = hPostProcessA,
            .PingPongB = hPostProcessB,
        });

        RecordFeatures(graph, frameData);

        if (postProcess.ActiveColor != postProcess.FinalTarget)
        {
            gfx::GFXTexture2DView_sp fallbackView = camRenderTexture.GetRenderTarget0();
            graph.AddPass("PostProcess_CopyToFinal")
                .Read(postProcess.ActiveColor)
                .Write(postProcess.FinalTarget)
                .NoRenderPass()
                .Execute([hSrc = postProcess.ActiveColor, hFinal = postProcess.FinalTarget, fallbackView](RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
                {
                    const auto* srcRT = passCtx.Get(hSrc);
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
    }
}
