#include "DisplayEncodingRenderFeature.h"
#include "BlitPass.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/DisplayEncodingSettings.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Rendering/SceneView.h>

namespace pulsar
{
    void DisplayEncodingRenderFeature::EnsureMaterial()
    {
        if (m_material)
            return;

        auto shader = AssetManager::Get()->LoadAsset<Shader>("Pulsar/Shaders/DisplayEncoding");
        if (!shader)
            return;

        m_material = Material::StaticCreate(shader);
        m_material->CreateGPUResource();
    }

    void DisplayEncodingRenderFeature::OnRecord(RenderGraph& graph, RenderFrameData& frameData)
    {
        auto* view = frameData.Get<ViewFrameData>();
        auto* postProcess = frameData.Get<PostProcessFrameData>();
        if (!view || !view->ViewData || !postProcess)
            return;

        auto* settings = view->ViewData->PostProcessStack.GetComponent<DisplayEncodingSettings>();
        if (!settings || !settings->m_enabled)
            return;

        EnsureMaterial();
        if (!m_material)
            return;

        m_material->SubmitParameters();

        auto proxy = m_material->GetRenderProxy();
        if (!proxy)
            return;

        auto destination = postProcess->AcquireTarget();
        auto* gpu = frameData.Get<GpuFrameData>();
        auto result = BlitPass::AddToGraph(graph, {
            .Name = "PostProcess_DisplayEncoding",
            .Source = postProcess->ActiveColor,
            .Destination = destination,
            .Material = std::move(proxy),
            .ShaderPass = "PostProcess",
            .Interface = "RENDERER_IMAGEPROCESS",
            .SourceBindingName = "PP_InColor",
            .CameraBuffer = gpu ? gpu->CameraBuffer : nullptr,
            .WorldBuffer = gpu ? gpu->WorldBuffer : nullptr,
        });
        postProcess->PushColor(result);
    }
}