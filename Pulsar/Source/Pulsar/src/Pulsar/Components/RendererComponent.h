#pragma once
#include "Component.h"
#include <Pulsar/Rendering/RenderObject.h>

namespace pulsar
{
    class Material;

    class IRendererComponent : public IInterface
    {
        CORELIB_DEF_INTERFACE(AssemblyObject_pulsar, pulsar::IRendererComponent, IInterface);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);
    public:
        virtual SPtr<rendering::RenderObject> CreateRenderObject() = 0;
    };

    class RendererComponent : public Component, public IRendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::RendererComponent, Component);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);
        CORELIB_IMPL_INTERFACES(IRendererComponent);
    public:
        RendererComponent() : CORELIB_INIT_INTERFACE(IRendererComponent) {}
    };
    DECL_PTR(RendererComponent);
}