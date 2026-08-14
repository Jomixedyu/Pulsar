#include "CustomPostProcessRenderModule.h"
#include "../Passes/BlitPass.h"
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Rendering/SceneView.h>
#include <string>

namespace pulsar
{
    void CustomPostProcessRenderModule::OnRecord(RenderGraph& graph, RenderFrameData& frameData)
    {
        auto* capture = frameData.Get<SceneCaptureFrameData>();
        auto* postProcess = frameData.Get<ScenePostProcessFrameData>();
        if (!capture || !capture->view || !postProcess)
            return;

        auto* gpu = frameData.Get<SceneCaptureGpuFrameData>();
        for (size_t index = 0; index < capture->view->PostProcessMaterials.size(); ++index)
        {
            auto& material = capture->view->PostProcessMaterials[index];
            if (!material)
                continue;

            material->SubmitParameters();
            auto proxy = material->GetRenderProxy();
            if (!proxy)
                continue;

            auto destination = postProcess->AcquireTarget();
            auto result = BlitPass::AddToGraph(graph, {
                .Name = std::string("PostProcess_Custom_") + std::to_string(index),
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
}