#include "Component.h"
#include <Apatite/Components/Component.h>
#include <Apatite/Node.h>

namespace apatite
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