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
    public:
        CORELIB_DEF_INTERFACE(AssemblyObject_Pulsar, pulsar::IRendererComponent, IInterface);

        virtual sptr<rendering::RenderObject> CreateRenderObject() = 0;
    };

}