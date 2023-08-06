#include "Components/Component.h"
#include <Pulsar/Components/Component.h>
#include <Pulsar/Node.h>

namespace pulsar
{

    sptr<Node> Component::GetAttachedNode()
    {
        return m_attachedNode.lock();
    }
    sptr<Node> Component::GetOnwerNode()
    {
        return m_ownerNode.lock();
    }
    bool Component::EqualsComponentType(Type* type)
    {
        return this->GetType() == type;
    }

    void Component::OnTick(Ticker ticker)
    {
    }


}