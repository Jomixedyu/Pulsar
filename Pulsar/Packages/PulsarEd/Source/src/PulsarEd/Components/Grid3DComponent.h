#pragma once
#include "Assembly.h"
#include <Pulsar/Components/RenderComponent.h>

namespace pulsared
{
    class Grid3DComponent : public RenderComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::Grid3DComponent, RenderComponent);
    public:
        Grid3DComponent()
        {
            m_flags |= OF_NoPack;
        }
        virtual void BeginComponent() override;

        virtual SPtr<rendering::RenderProxy> CreateRenderProxy() override;
        virtual void EndComponent() override;

        void OnTransformChanged() override;
        void SyncRenderProxy() override;
    private:
        array_list<Vector3f> m_vert;
        array_list<Color4b> m_colors;

        SPtr<rendering::RenderObject> m_renderObject;
    };
    DECL_PTR(Grid3DComponent);
}