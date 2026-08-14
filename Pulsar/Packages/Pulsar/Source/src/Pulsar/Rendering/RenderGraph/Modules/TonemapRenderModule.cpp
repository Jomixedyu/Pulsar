#include "TonemapRenderModule.h"
#include "../Passes/BlitPass.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Assets/TonemappingSettings.h>
#include <Pulsar/Rendering/SceneView.h>

namespace pulsar
{
    void TonemapRenderModule::EnsureMaterial()
    {
        if (m_material)
            return;

        auto shader = AssetManager::Get()->LoadAsset<Shader>("Pulsar/Shaders/Tonemap");
        if (!shader)
            return;

        m_material = Material::StaticCreate(shader);
        m_material->CreateGPUResource();
    }

    void TonemapRenderModule::OnRecord(RenderGraph& graph, RenderFrameData& frameData)
    {
        auto* capture = frameData.Get<SceneCaptureFrameData>();
        auto* postProcess = frameData.Get<ScenePostProcessFrameData>();
        if (!capture || !capture->view || !postProcess)
            return;

        auto* settings = capture->view->PostProcessStack.GetComponent<TonemappingSettings>();
        if (!settings || !settings->m_enabled)
            return;

        EnsureMaterial();
        if (!m_material)
            return;

        m_material->SetIntScalar("_TonemappingMode", static_cast<int>(settings->m_mode));
        m_material->SubmitParameters();

        auto proxy = m_material->GetRenderProxy();
        if (!proxy)
            return;

        auto destination = postProcess->AcquireTarget();
        auto* gpu = frameData.Get<SceneCaptureGpuFrameData>();
        auto result = BlitPass::AddToGraph(graph, {
            .Name = "PostProcess_Tonemap",
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