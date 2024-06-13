#include "Physics2D/CircleCollider2DComponent.h"
#include <box2d/b2_circle_shape.h>

namespace pulsar
{

    void CircleCollider2DComponent::SetRadius(float value)
    {
        m_radius = value;
    }
} // namespace pulsar