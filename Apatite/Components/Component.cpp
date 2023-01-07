#include <Apatite/Components/Component.h>
#include <Apatite/Node.h>

namespace apatite
{

    bool Component::EqualsComponentType(Type* type)
    {
        return this->GetType() == type;
    }

    void Component::OnTick(Ticker ticker)
    {
    }


}