#include "Components/SceneCaptureComponent.h"
#include "Pulsar/Rendering/RenderGraph/DefaultSceneCaptureRenderer.h"
#include "World.h"

namespace pulsar
{

    SceneCaptureComponent::SceneCaptureComponent()
    {
        m_captureRenderer = std::make_unique<DefaultSceneCaptureRenderer>();
    }

    void SceneCaptureComponent::BeginComponent()
    {
        base::BeginComponent();
        GetWorld()->GetCaptureManager().Add(this);
    }
    void SceneCaptureComponent::EndComponent()
    {
        m_captureRenderer.reset();
        base::EndComponent();
        GetWorld()->GetCaptureManager().Remove(this);
    }



}