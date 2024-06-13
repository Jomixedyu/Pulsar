#pragma once
#include "Collider2DComponent.h"

namespace pulsar
{
    class BoxCollider2DComponent : public Collider2DComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BoxCollider2DComponent, Collider2DComponent);

    public:
        BoxCollider2DComponent();

        Vector2f GetSize() const { return m_size; }
        void SetSize(Vector2f value);

    protected:
        Vector2f m_size;
    };
}