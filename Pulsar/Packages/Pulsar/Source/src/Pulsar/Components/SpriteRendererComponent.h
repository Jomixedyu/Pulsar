#pragma once
#include <Pulsar/IconsForkAwesome.h>
#include "Assets/SpriteAtlas.h"
#include "RenderComponent.h"

namespace pulsar
{
    class SpriteRendererComponent : public RenderComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SpriteRendererComponent, RenderComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Renderer"), new ComponentIconAttribute(ICON_FK_PICTURE_O));
    public:
        SpriteRendererComponent() = default;

        void BeginComponent() override;
        void EndComponent() override;

        SPtr<rendering::RenderProxy> CreateRenderProxy() override;

        CORELIB_REFL_DECL_FIELD(m_material);
        RCPtr<Material> m_material;

        CORELIB_REFL_DECL_FIELD(m_spriteAtlas);
        RCPtr<SpriteAtlas> m_spriteAtlas;

        CORELIB_REFL_DECL_FIELD(m_spriteName);
        string m_spriteName;

    };
}