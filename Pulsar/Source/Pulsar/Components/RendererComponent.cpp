#include "RendererComponent.h"

namespace pulsar
{
    void RendererComponent::OnConstruct()
    {
        this->materials_ = mksptr(new List<Material_sp>);
    }
}