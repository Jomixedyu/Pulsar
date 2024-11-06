#include "Components/SceneCaptureComponent.h"

#include "World.h"

namespace pulsar
{

    SceneCaptureComponent::SceneCaptureComponent()
    {
        init_sptr_member(m_postProcessMaterials);
    }

    void SceneCaptureComponent::BeginComponent()
    {
        base::BeginComponent();
        GetWorld()->GetCaptureManager().Add(this);
    }
    void SceneCaptureComponent::EndComponent()
    {
        base::EndComponent();
        GetWorld()->GetCaptureManager().Remove(this);
    }

    void SceneCaptureComponent::SetPostProcess(size_t index, const RCPtr<Material>& value) const
    {
        value->CreateGPUResource();
        m_postProcessMaterials->at(index) = value;
    }
    void SceneCaptureComponent::AddPostProcess(const RCPtr<Material>& material)
    {
        material->CreateGPUResource();
        m_postProcessMaterials->push_back(material);
    }
    void SceneCaptureComponent::RemovePostProcessAt(size_t index)
    {
        m_postProcessMaterials->RemoveAt((int32_t)index);
    }
    void SceneCaptureComponent::ClearPostProcess()
    {
        m_postProcessMaterials->clear();
    }

}