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
    SPtr<rendering::RenderObject> SpriteRendererComponent::CreateRenderObject()
    {
        return {};
    }

} // namespace pulsar