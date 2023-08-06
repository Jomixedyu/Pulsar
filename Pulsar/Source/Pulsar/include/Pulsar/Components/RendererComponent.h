#pragma once

#include "Component.h"
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Rendering/RenderObject.h>

namespace pulsar
{
    class Mesh;
    class Material;
    class ShaderPass;

    class RendererComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::RendererComponent, Component)
    public:
        virtual sptr<rendering::RenderObject> CreateRenderObject() = 0;




    };
    DECL_PTR(RendererComponent);
}