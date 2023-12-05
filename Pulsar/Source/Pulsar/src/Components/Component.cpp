#include "Components/Component.h"
#include <Pulsar/Components/Component.h>
#include <Pulsar/Rendering/RenderObject.h>
#include <Pulsar/Components/RendererComponent.h>
#include <Pulsar/World.h>
#include <Pulsar/Node.h>

namespace pulsar
{

    ObjectPtr<Node> Component::GetAttachedNode() const
    {
        return m_attachedNode;
    }
    ObjectPtr<Node> Component::GetOwnerNode() const
    {
        return m_ownerNode;
    }
    World* Component::GetWorld() const
    {
        return GetAttachedNode()->GetRuntimeWorld();
    }
    void Component::OnReceiveMessage(MessageId id)
    {
        if(id == MessageId_OnChangedTransform)
        {
            OnMsg_TransformChanged();
        }
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
        m_beginning = true;
    }
    void Component::EndComponent()
    {
        m_beginning = false;
    }
} // namespace pulsar