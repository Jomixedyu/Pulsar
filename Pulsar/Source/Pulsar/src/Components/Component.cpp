#include "Components/Component.h"
#include <Pulsar/Components/Component.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Components/RendererComponent.h>
#include <Pulsar/World.h>
#include <Pulsar/Node.h>

namespace pulsar
{

    ObjectPtr<Node> Component::GetAttachedNode()
    {
        return m_attachedNode;
    }
    ObjectPtr<Node> Component::GetOnwerNode()
    {
        return m_ownerNode;
    }
    bool Component::EqualsComponentType(Type* type)
    {
        return this->GetType() == type;
    }

    void Component::OnTick(Ticker ticker)
    {
    }

    void Component::BeginComponent()
    {

    }
}