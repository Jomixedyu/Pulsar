#include "TranslucencyRenderFeature.h"
#include <Pulsar/Rendering/RenderGraph/Modules/TranslucencyRenderModule.h>
#include <Pulsar/Rendering/RenderGraph/Pipelines/SceneCaptureFrameData.h>
#include <Pulsar/Rendering/RenderGraph/RenderGraph.h>
#include <Pulsar/Rendering/SceneView.h>
#include <gfx/TextureClasses.h>
#include <gfx/GFXCommandBuffer.h>

namespace pulsar
{
    class TranslucencyRenderFeature final : public RenderFeature
    {
    public:
        void OnCreateResource() override { m_module.Initialize(); }
        void OnDestroyResource() override { m_module.Destroy(); }
        void OnRecord(RenderGraph& graph, RenderFrameData& frameData) override
        {
            auto* sceneTarget = frameData.Get<SceneRenderTargetFrameData>();
            auto* capture = frameData.Get<SceneCaptureFrameData>();
            if (!sceneTarget || !capture || !capture->view)
                return;

            RGTextureDesc opaqueColorDesc{};
            opaqueColorDesc.Width = capture->view->RenderTarget.Width;
            opaqueColorDesc.Height = capture->view->RenderTarget.Height;
            opaqueColorDesc.TargetInfos.push_back({ gfx::GFXTextureTargetType::ColorTarget, gfx::GFXTextureFormat::R16G16B16A16_SFloat });
            auto& opaqueColor = frameData.Set(SceneOpaqueColorFrameData{ graph.CreateTransient("OpaqueColorTexture", opaqueColorDesc) });

            graph.AddPass("CopyOpaqueColor")
                .Read(sceneTarget->Target)
                .Write(opaqueColor.Color)
                .NoRenderPass()
                .Execute([hSceneColor = sceneTarget->Target, hOpaqueColor = opaqueColor.Color](RGPassContext& passCtx, gfx::GFXCommandBuffer& cmdBuffer)
                {
                    auto* srcRT = passCtx.Get(hSceneColor);
                    auto* dstRT = passCtx.Get(hOpaqueColor);
                    if (!srcRT || !dstRT) return;
                    auto srcView = srcRT->GetRenderTarget0();
                    auto dstView = dstRT->GetRenderTarget0();
                    if (!srcView || !dstView) return;
                    cmdBuffer.CmdBlit(srcView.get(), dstView.get());
                });

            m_module.OnRecord(graph, frameData);
        }
    private:
        TranslucencyRenderModule m_module;
    };

    RenderFeatureFactory TranslucencyRenderFeatureSettings::BuildRenderFeatureFactory() const
    {
        return []() -> std::unique_ptr<RenderFeature>
        {
            return std::make_unique<TranslucencyRenderFeature>();
        };
    }
}