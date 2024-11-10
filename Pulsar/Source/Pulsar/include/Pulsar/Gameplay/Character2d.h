#pragma once
#include "Components/Component.h"
#include "InputSystem/InputComponent.h"

namespace pulsar
{

    class Character2d : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Character2d, Component);
        CORELIB_CLASS_ATTR(new CategoryAttribute("2D"))
    public:
        void BeginPlay() override;
        void EndPlay() override;
        void OnTick(Ticker ticker) override;


        void OnInput(SPtr<InputContext> ctx);

    protected:
        InputComponent_ref inputComponent;
    };
} // namespace pulsar
