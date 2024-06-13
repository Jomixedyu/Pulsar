#include "Physics2D/BoxCollider2DComponent.h"

namespace pulsar
{

    BoxCollider2DComponent::BoxCollider2DComponent()
        : m_size({1, 1})
    {
    }
    void BoxCollider2DComponent::SetSize(Vector2f value)
    {
        m_size = value;
    }
} // namespace pulsar