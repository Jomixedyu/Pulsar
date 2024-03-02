#pragma once
#include "EditorComponent.h"
#include <Pulsar/Components/RendererComponent.h>

namespace pulsared
{
    class Grid3DComponent : public EditorComponent, public IRendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::Grid3DComponent, EditorComponent);
        CORELIB_IMPL_INTERFACES(IRendererComponent);
    public:
        Grid3DComponent() : CORELIB_INIT_INTERFACE(IRendererComponent)
        {

        }
        virtual void BeginComponent() override;

        virtual sptr<rendering::RenderObject> CreateRenderObject() override;
        virtual void EndComponent() override;

        void OnMsg_TransformChanged() override;
    private:
        array_list<Vector3f> m_vert;
        array_list<Color4f> m_colors;

        sptr<rendering::RenderObject> m_renderObject;
    };
    DECL_PTR(Grid3DComponent);
}