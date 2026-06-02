#pragma once
#include <Pulsar/IconsForkAwesome.h>
#include "CanvasComponent.h"

namespace pulsar::ui
{
    class ButtonComponent : public CanvasElement
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ui::ButtonComponent, CanvasElement);
        CORELIB_CLASS_ATTR(new CategoryAttribute("UI"), new ComponentIconAttribute(ICON_FK_DESKTOP));
    public:

    };
    DECL_PTR(ButtonComponent);

}