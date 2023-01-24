#include "RendererComponent.h"

namespace apatite
{
    void RendererComponent::OnConstruct()
    {
        this->materials_ = mksptr(new List<Material_sp>);
    }
}