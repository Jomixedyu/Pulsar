#include "Components/RenderComponent.h"

#include <Pulsar/World.h>

namespace pulsar
{
    void RenderComponent::BeginComponent()
    {
        base::BeginComponent();
        if (auto* world = GetWorld())
            world->RegisterProxy(this);
    }

    void RenderComponent::EndComponent()
    {
        m_renderStateDirty = false;
        if (auto* world = GetWorld())
            world->UnregisterProxy(this);
        base::EndComponent();
    }

    void RenderComponent::MarkRenderStateDirty()
    {
        if (m_renderStateDirty)
            return;
        m_renderStateDirty = true;
        if (auto* world = GetWorld())
            world->MarkRenderStateDirty(this);
    }
}
