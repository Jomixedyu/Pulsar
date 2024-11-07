#pragma once
#include "Assets/SpriteAtlas.h"
#include "RendererComponent.h"

namespace pulsar
{
    class SpriteRendererComponent : public Component, public IRendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SpriteRendererComponent, Component);
        CORELIB_IMPL_INTERFACES(IRendererComponent);
    public:
        SpriteRendererComponent() : CORELIB_INIT_INTERFACE(IRendererComponent)
        {

        }

        SPtr<rendering::RenderObject> CreateRenderObject() override;

        RCPtr<Material> m_material;
        RCPtr<SpriteAtlas> m_spriteAtlas;

    };
}