#pragma once
#include "Component.h"

namespace pulsar
{
    class MeshRendererComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::MeshRendererComponent, Component);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);
    public:

    };
    DECL_PTR(MeshRendererComponent);
}