#pragma once
#include "Component.h"
#include <Pulsar/Assets/StaticMesh.h>

namespace pulsar
{
    class MeshRendererComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::MeshRendererComponent, Component);
    public:

    };
    DECL_PTR(MeshRendererComponent);
}