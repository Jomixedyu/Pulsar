#pragma once

#include "Component.h"
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Rendering/RenderObject.h>

namespace pulsar
{
    class Mesh;
    class Material;
    class ShaderPass;
    
    class IRendererComponent : public IInterface
    {
        CORELIB_DEF_INTERFACE(AssemblyObject_pulsar, pulsar::IRendererComponent, IInterface);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);
    public:
        virtual sptr<rendering::RenderObject> CreateRenderObject() = 0;


    };

}