#pragma once

#include <Pulsar/ObjectBase.h>
#include <Pulsar/Assets/Material.h>
#include "Component.h"


namespace pulsar
{
    class Mesh;
    class Material;
    class ShaderPass;

    class IRenderObject
    {
    public:

    };

    class RendererComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::RendererComponent, Component)
    public:
        virtual sptr<IRenderObject> CreateRenderObject() = 0;


    };
    CORELIB_DECL_SHORTSPTR(RendererComponent);
}