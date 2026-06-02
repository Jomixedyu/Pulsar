#pragma once
#include "Component.h"
#include <Pulsar/Input.h>
#include <unordered_map>

namespace pulsar
{
    class InputComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputComponent, Component);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Input"))
    public:
        void BeginPlay() override;
        void OnTick(Ticker ticker) override;

        // --- Action queries ---
        bool GetAction(string_view name) const;
        bool GetActionDown(string_view name) const;
        bool GetActionUp(string_view name) const;

        // --- Axis query ---
        float GetAxis(string_view name) const;

        // --- Configuration (code-driven binding) ---
        void BindAction(string_view name, KeyCode key);
        void BindAxis(string_view name, KeyCode negativeKey, KeyCode positiveKey, float scale = 1.0f);

    private:
        struct ActionState
        {
            KeyCode key;
            bool current = false;
            bool previous = false;
        };
        struct AxisState
        {
            KeyCode negativeKey;
            KeyCode positiveKey;
            float scale = 1.0f;
            float value = 0.0f;
        };

        std::unordered_map<index_string, ActionState> m_actions;
        std::unordered_map<index_string, AxisState> m_axes;
    };
}
