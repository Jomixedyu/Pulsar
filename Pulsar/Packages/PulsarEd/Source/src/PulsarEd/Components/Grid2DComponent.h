#pragma once
#include "Assembly.h"
#include <Pulsar/Components/RenderComponent.h>

namespace pulsared
{
    class Grid2DComponent : public RenderComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::Grid2DComponent, RenderComponent);
    public:
        Grid2DComponent()
        {
            m_flags |= OF_NoPack;
        }
        virtual void BeginComponent() override;

        virtual SPtr<rendering::RenderProxy> CreateRenderProxy() override;
        virtual void EndComponent() override;

        void OnTransformChanged() override;
        void ResolveRenderStateDirty() override;
    private:
        array_list<Vector3f> m_vert;
        array_list<Color4b> m_colors;

        SPtr<rendering::RenderObject> m_renderObject;
    };
    DECL_PTR(Grid2DComponent);
}