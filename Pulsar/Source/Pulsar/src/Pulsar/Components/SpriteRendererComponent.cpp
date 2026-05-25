#include "Components/SpriteRendererComponent.h"

namespace pulsar
{
    void SpriteRendererComponent::BeginComponent()
    {
        base::BeginComponent();
    }
    void SpriteRendererComponent::EndComponent()
    {
        base::EndComponent();
    }
    SPtr<rendering::RenderProxy> SpriteRendererComponent::CreateRenderObject()
    {
        return {};
    }

} // namespace pulsar