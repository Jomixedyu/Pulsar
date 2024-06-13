#pragma once
#include "Collider2DComponent.h"

namespace pulsar
{
    class CircleCollider2DComponent : public Collider2DComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CircleCollider2DComponent, Collider2DComponent);

    public:

        float GetRadius() const { return m_radius; }
        void SetRadius(float value);

    protected:

        float m_radius = 1;
    };
}