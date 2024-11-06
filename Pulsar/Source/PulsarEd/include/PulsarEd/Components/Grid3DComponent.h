#pragma once
#include "Assembly.h"
#include <Pulsar/Components/RendererComponent.h>

namespace pulsared
{
    class Grid3DComponent : public Component, public IRendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::Grid3DComponent, Component);
        CORELIB_IMPL_INTERFACES(IRendererComponent);
    public:
        Grid3DComponent() : CORELIB_INIT_INTERFACE(IRendererComponent)
        {
            m_flags |= OF_NoPack;
        }
        virtual void BeginComponent() override;

        virtual SPtr<rendering::RenderObject> CreateRenderObject() override;
        virtual void EndComponent() override;

        void OnTransformChanged() override;
    private:
        array_list<Vector3f> m_vert;
        array_list<Color4f> m_colors;

        SPtr<rendering::RenderObject> m_renderObject;
    };
    DECL_PTR(Grid3DComponent);
}