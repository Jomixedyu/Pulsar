#include "Components/SceneCaptureComponent.h"
#include "World.h"
#include <Pulsar/Rendering/SceneView.h>
#include <Pulsar/Subsystems/PostProcessSubsystem.h>


namespace pulsar
{

    SceneCaptureComponent::SceneCaptureComponent()
    {
    }

    SPtr<rendering::RenderProxy> SceneCaptureComponent::CreateRenderProxy()
    {
        auto view = mksptr(new SceneView());
        if (m_viewPipeline)
            m_viewPipeline->BuildRenderData(view->Data.ViewPipeline);
        m_sceneView = view;
        return view;
    }

    void SceneCaptureComponent::SyncRenderProxy()
    {
        if (!m_sceneView)
            return;

        SceneViewData data{};
        if (!ExtractViewData(data))
            return;


        if (m_viewPipeline)
            m_viewPipeline->BuildRenderData(data.ViewPipeline);


        // Snapshot post-process settings on the game thread for this view's camera
        // position, so the render thread reads the blended stack instead of querying
        // the live PostProcessSubsystem.
        if (auto* ppSub = GetWorld()->GetSubsystem<PostProcessSubsystem>())
        {
            data.PostProcessStack = ppSub->QuerySettings(data.CameraPosition);
            data.PostProcessMaterials = ppSub->QueryPostProcessMaterials(data.CameraPosition);
        }

        GetWorld()->UpdateSceneView(m_sceneView, std::move(data));
    }

    void SceneCaptureComponent::BeginComponent()
    {
        base::BeginComponent();
        GetWorld()->GetCaptureManager().Add(this);
        // Push an initial snapshot to the freshly-created view proxy.
        MarkRenderStateDirty();
    }
    void SceneCaptureComponent::EndComponent()
    {
        m_sceneView.reset();
        base::EndComponent();
        GetWorld()->GetCaptureManager().Remove(this);
    }



}
