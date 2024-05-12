#pragma once
#include "Collider2DComponent.h"

namespace pulsar
{
    class PolygonCollider2DComponent : public Collider2DComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::PolygonCollider2DComponent, Collider2DComponent);

    public:

    protected:

        array_list<Vector2f> m_points;
    };
}