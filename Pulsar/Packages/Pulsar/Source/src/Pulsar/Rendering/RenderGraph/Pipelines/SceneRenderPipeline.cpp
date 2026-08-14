#include "SceneRenderPipeline.h"
#include "SceneCaptureFrameData.h"
#include "ViewPipeline.h"
#include <Pulsar/Rendering/LightProxy.h>
#include <Pulsar/Rendering/RenderScene.h>
#include <Pulsar/Rendering/SceneView.h>

namespace pulsar
{
    void SceneRenderPipeline::OnRecord(RenderGraph& graph, RenderScene& scene, uint64_t frameIndex)
    {
        auto& perRenderObjectData = scene.GetPerRenderObjectData();
        perRenderObjectData.BeginFrame();

        {
            PerPassWorldData worldData{};
            worldData.TotalTime = scene.GetTotalTime();
            worldData.DeltaTime = scene.GetDeltaTime();

            const DirectionalLightProxy* brightest = nullptr;
            for (const auto& directionalLight : scene.GetDirectionalLights())
            {
                if (!brightest || directionalLight->Intensity > brightest->Intensity)
                    brightest = directionalLight.get();
            }
            if (brightest)
            {
                worldData.WorldSpaceLightVector = -brightest->Vector;
                const auto& color = brightest->Color;
                worldData.WorldSpaceLightColor = {color.r, color.g, color.b, brightest->Intensity};
            }
            worldData.SkyLightColor = {0, 0, 0, 0};
            worldData.LightParameterCount = static_cast<uint32_t>(scene.GetPointLights().size());
            scene.UploadWorld(worldData);
        }

        {
            PerPassLightsBufferData lightsData{};
            const auto& pointLights = scene.GetPointLights();
            const int lightCount = std::min(static_cast<int>(pointLights.size()), 63);
            for (int index = 0; index < lightCount; ++index)
                lightsData.Lights[index] = pointLights[index]->Param;
            scene.UploadLights(lightsData);
        }

        for (const auto& view : scene.GetViews())
        {
            if (!view || !view->Pipeline)
                continue;

            PerPassCameraData cameraData{};
            cameraData.MatrixV = view->Data.ViewMatrix;
            cameraData.MatrixP = view->Data.ProjectionMatrix;
            cameraData.MatrixVP = cameraData.MatrixP * cameraData.MatrixV;
            cameraData.InvMatrixV = jmath::Inverse(cameraData.MatrixV);
            cameraData.InvMatrixP = jmath::Inverse(cameraData.MatrixP);
            cameraData.InvMatrixVP = jmath::Inverse(cameraData.MatrixVP);
            cameraData.CamPosition = Vector4f(view->Data.CameraPosition, 1.f);
            cameraData.CamNear = view->Data.Near;
            cameraData.CamFar = view->Data.Far;
            cameraData.Resolution = view->Data.Resolution;
            view->UploadCamera(cameraData);


            RenderFrameData frameData;
            frameData.Set(SceneCaptureFrameData{
                .view = &view->Data,
                .viewProxy = view.get(),
                .scene = &scene,
                .frameIndex = frameIndex,
            });
            view->Pipeline->OnRecord(graph, frameData);
        }

        perRenderObjectData.EndFrame();
    }
}
