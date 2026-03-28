#pragma once
#include "Assets/SpriteAtlas.h"
#include "RendererComponent.h"

namespace pulsar
{
    class SpriteRendererComponent : public RendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SpriteRendererComponent, RendererComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Renderer"))
    public:
        SpriteRendererComponent() = default;

        void BeginComponent() override;
        void EndComponent() override;

        SPtr<rendering::RenderObject> CreateRenderObject() override;

        CORELIB_REFL_DECL_FIELD(m_material);
        RCPtr<Material> m_material;

        CORELIB_REFL_DECL_FIELD(m_spriteAtlas);
        RCPtr<SpriteAtlas> m_spriteAtlas;

        CORELIB_REFL_DECL_FIELD(m_spriteName);
        string m_spriteName;

    };
}