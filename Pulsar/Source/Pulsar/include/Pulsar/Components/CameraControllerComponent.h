#pragma once
#include "Component.h"

namespace pulsar
{

    class CameraControllerComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CameraControllerComponent, Component);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);

    public:

        virtual void CanRotate();

    };

} // namespace pulsar