#pragma once
#include "Components/Component.h"

namespace pulsar
{
    class Movement3d : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Movement3d, Component);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Gameplay"))
    public:
        void BeginPlay() override;
        void EndPlay() override;
        void OnTick(Ticker ticker) override;

    private:
        CORELIB_REFL_DECL_FIELD(m_speed);
        float m_speed = 3.0f;
    };
}
