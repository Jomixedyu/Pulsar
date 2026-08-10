#include "CustomPostProcessRenderFeature.h"
#include "BlitPass.h"
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Rendering/SceneView.h>
#include <string>

namespace pulsar
{
    void CustomPostProcessRenderFeature::OnRecord(RenderGraph& graph, RenderFrameData& frameData)
    {
        auto* view = frameData.Get<ViewFrameData>();
        auto* postProcess = frameData.Get<PostProcessFrameData>();
        if (!view || !view->ViewData || !postProcess)
            return;

        auto* gpu = frameData.Get<GpuFrameData>();
        for (size_t index = 0; index < view->ViewData->PostProcessMaterials.size(); ++index)
        {
            auto& material = view->ViewData->PostProcessMaterials[index];
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