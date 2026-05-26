#pragma once
#include <Pulsar/EngineMath.h>
#include <uinput/InputKeyCode.h>
#include <uinput/InputManager.h>
#include <set>

namespace pulsar
{
    using KeyCode = uinput::KeyCode;

    // Pure input state machine.  No viewport awareness; the caller is
    // responsible for routing events and converting coordinates.
    struct InputContext
    {
    public:
        // --- Lifecycle ---
        void BeginFrame();  // Snapshot previous frame state
        void ProcessEvent(const uinput::InputEvent& e);

        // --- Query APIs ---
        bool GetKey(KeyCode key) const;
        bool GetKeyDown(KeyCode key) const;
        bool GetKeyUp(KeyCode key) const;

        Vector2f GetMousePosition() const;     // local to the caller's viewport
        Vector2f GetMouseDelta() const;

        bool GetMouseButton(int button) const;
        bool GetMouseButtonDown(int button) const;
        bool GetMouseButtonUp(int button) const;

        float GetMouseScrollWheel() const;
        float GetMouseScrollWheelDelta() const;

    private:
        std::set<KeyCode> currentKeys;
        std::set<KeyCode> previousKeys;
        std::set<int> currentMouseButtons;
        std::set<int> previousMouseButtons;
        Vector2f mousePosition;
        Vector2f prevMousePosition;
        float mouseWheel = 0.0f;
        float prevMouseWheel = 0.0f;
    };
}
