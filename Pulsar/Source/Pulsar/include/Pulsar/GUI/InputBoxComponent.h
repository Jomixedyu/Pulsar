#pragma once
#include "CanvasComponent.h"

namespace pulsar::ui
{
    class InputBoxComponent : public CanvasElement
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ui::InputBoxComponent, CanvasElement);
        CORELIB_CLASS_ATTR(new CategoryAttribute("UI"));
    public:

    };
    DECL_PTR(InputBoxComponent);

}