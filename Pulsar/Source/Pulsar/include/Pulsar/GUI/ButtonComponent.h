#pragma once
#include "CanvasComponent.h"

namespace pulsar::ui
{
    class ButtonComponent : public CanvasElement
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ui::ButtonComponent, CanvasElement);
        CORELIB_CLASS_ATTR(new CategoryAttribute("UI"));
    public:

    };
    DECL_PTR(ButtonComponent);

}