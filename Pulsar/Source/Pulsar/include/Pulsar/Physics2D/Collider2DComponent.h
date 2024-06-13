#pragma once
#include <Pulsar/Components/Component.h>

namespace pulsar
{
    struct Collision2D
    {
        ObjectPtr<Node> node;
    };

    class Collider2DComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Collider2DComponent, Component);
    public:

        bool GetIsTrigger() const { return m_isTrigger; }
        void SetIsTrigger(bool value);
    public:
        ActionEvents<const Collision2D&> OnCollisionEnter2D;
        ActionEvents<const Collision2D&> OnCollisionExit2D;
        ActionEvents<const Collision2D&> OnTriggerEnter2D;
        ActionEvents<const Collision2D&> OnExitEnter2D;
    protected:
        bool m_isTrigger{};
        Vector2f m_offset{};
    };
}