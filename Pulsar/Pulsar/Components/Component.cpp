#include "Component.h"
#include <Pulsar/Components/Component.h>
#include <Pulsar/Node.h>

namespace pulsar
{
    sptr<Node> Component::get_node()
    {
        if (this->node_.expired())
        {
            return nullptr;
        }
        return this->node_.lock();
    }
    bool Component::EqualsComponentType(Type* type)
    {
        return this->GetType() == type;
    }

    void Component::OnTick(Ticker ticker)
    {
    }


}