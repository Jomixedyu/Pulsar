#pragma once

#include "Component.h"


namespace pulsar
{
    class PlayableComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::PlayableComponent, Component);
    public:
        bool GetInteractivePlaying() const { return m_enabled; }
        void SetInteractivePlaying(bool value) { m_enabled = value;}

    public:
        virtual void BeginPlay() {}
        virtual void Tick(float dt) {}
        virtual void EndPlay() {}

    private:
        bool m_enabled{};
    };
}