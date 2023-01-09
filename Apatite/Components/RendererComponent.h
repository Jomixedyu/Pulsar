#pragma once

#include <Apatite/ObjectBase.h>
#include "Component.h"


namespace apatite
{
    class Mesh;
    class Material;
    class ShaderProgram;

    class RendererComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::RendererComponent, Component)
    public:

    };
    CORELIB_DECL_SHORTSPTR(RendererComponent);
}