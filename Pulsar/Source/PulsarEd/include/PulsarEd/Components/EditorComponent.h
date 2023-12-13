#pragma once
#include <Pulsar/Components/Component.h>
#include <PulsarEd/Assembly.h>

namespace pulsared
{
    class EditorComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::EditorComponent, Component);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);
    public:
        EditorComponent();
    };

}