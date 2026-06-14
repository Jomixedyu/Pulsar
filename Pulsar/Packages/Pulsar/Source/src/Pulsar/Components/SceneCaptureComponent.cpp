#include "Components/SceneCaptureComponent.h"
#include "World.h"
#include <Pulsar/Rendering/SceneView.h>
#include <Pulsar/Rendering/RenderGraph/DefaultSceneCaptureRenderer.h>

namespace pulsar
{

    SceneCaptureComponent::SceneCaptureComponent()
    {
    }

    SPtr<rendering::RenderProxy> SceneCaptureComponent::CreateRenderProxy()
    {
        auto view = mksptr(new SceneView());
        view->Renderer = std::make_unique<DefaultSceneCaptureRenderer>();
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
