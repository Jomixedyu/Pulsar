#include "Components/Shape2DComponent.h"

namespace pulsar
{
    Shape2DComponent::Shape2DComponent()
    {
        m_canDrawGizmo = true;
    }

    void Shape2DComponent::SetIsSensor(bool v)
    {
        if (m_isSensor == v)
            return;
        m_isSensor = v;
        ++m_shapeVersion;
    }
} // namespace pulsar
