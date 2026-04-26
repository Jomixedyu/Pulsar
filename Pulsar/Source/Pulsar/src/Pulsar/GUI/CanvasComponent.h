#pragma once
#include <Pulsar/Components/Component.h>

namespace pulsar::ui
{

    class CanvasElement : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ui::CanvasElement, Component);
    public:

        virtual Vector2f GetCanvasElementBounds() { return {}; }
        virtual bool HasCanvasElementBounds() { return false; }

    };

    class CanvasComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ui::CanvasComponent, Component);
        CORELIB_CLASS_ATTR(new CategoryAttribute("UI"));

    public:
        CanvasComponent();

        void BeginComponent() override;
        void EndComponent() override;

    };
    DECL_PTR(CanvasComponent);


}