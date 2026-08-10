#include "ColorGradingRenderModule.h"
#include "../Passes/BlitPass.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/ColorGradingSettings.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Rendering/SceneView.h>

namespace pulsar
{
    void ColorGradingRenderModule::EnsureMaterial()
    {
        if (m_material)
            return;

        auto shader = AssetManager::Get()->LoadAsset<Shader>("Pulsar/Shaders/LUT");
        if (!shader)
            return;

        m_material = Material::StaticCreate(shader);
        m_material->CreateGPUResource();
    }

    void ColorGradingRenderModule::OnRecord(RenderGraph& graph, RenderFrameData& frameData)
    {
        auto* view = frameData.Get<ViewFrameData>();
        auto* postProcess = frameData.Get<PostProcessFrameData>();
        if (!view || !view->ViewData || !postProcess)
            return;

        auto* settings = view->ViewData->PostProcessStack.GetComponent<ColorGradingSettings>();
        if (!settings || !settings->m_enabled || !settings->m_lutTexture)
            return;

        EnsureMaterial();
        if (!m_material)
            return;

        m_material->SetTexture("_LUTTex", settings->m_lutTexture);
        m_material->SetFloat("_Intensity", settings->m_intensity);
        m_material->SetIntScalar("_LUTSize", settings->m_lutSize);
        m_material->SetIntScalar("_ColorSpace", static_cast<int>(settings->m_colorSpace));
        m_material->SubmitParameters();

        auto proxy = m_material->GetRenderProxy();
        if (!proxy)
            return;

        auto destination = postProcess->AcquireTarget();
        auto* gpu = frameData.Get<GpuFrameData>();
        auto result = BlitPass::AddToGraph(graph, {
            .Name = "PostProcess_LUT",
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