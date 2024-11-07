#pragma once
#include "Shape3DComponent.h"

namespace pulsar
{
    class BoxShape3DComponent : public Shape3DComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BoxShape3DComponent, Shape3DComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("3D"));
    public:

    };
} // namespace pulsar