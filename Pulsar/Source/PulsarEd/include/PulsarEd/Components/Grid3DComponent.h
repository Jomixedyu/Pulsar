#pragma once
#include "EditorComponent.h"
#include <Pulsar/Components/RendererComponent.h>

namespace pulsared
{
    class Grid3DComponent : public EditorComponent, public IRendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_PulsarEd, pulsared::Grid3DComponent, EditorComponent);
        CORELIB_IMPL_INTERFACES(IRendererComponent);
    public:
        Grid3DComponent() : CORELIB_INIT_INTERFACE(IRendererComponent)
        {

        }
        virtual void OnInitialize() override;

        virtual sptr<rendering::RenderObject> CreateRenderObject() override;
    };
}