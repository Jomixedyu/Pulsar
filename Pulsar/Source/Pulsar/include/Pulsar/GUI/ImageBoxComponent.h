#pragma once
#include "Assets/SpriteAtlas.h"
#include "CanvasComponent.h"

namespace pulsar::ui
{
    class ImageBoxComponent : public CanvasElement
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ui::ImageBoxComponent, CanvasElement);
        CORELIB_CLASS_ATTR(new CategoryAttribute("UI"));
    public:


        CORELIB_REFL_DECL_FIELD(m_sprite);
        SPtr<SpriteAtlasReference> m_sprite;

    };
    DECL_PTR(ImageBoxComponent);

}