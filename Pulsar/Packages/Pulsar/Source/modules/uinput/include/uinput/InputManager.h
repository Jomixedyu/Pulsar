#pragma once
#include <vector>
#include "InputKeyCode.h"

namespace uinput
{
    struct InputVector2f
    {
        float x = 0.0f;
        float y = 0.0f;
    };

    // Unified platform-agnostic input event
    struct InputEvent
    {
        enum Type
        {
            KeyDown,
            KeyUp,
            MouseMove,
            MouseButtonDown,
            MouseButtonUp,
            MouseWheel,
        };

        Type type;

        // Key events
        KeyCode keyCode = static_cast<KeyCode>(0);

        // Mouse move
        float mouseX = 0.0f;
        float mouseY = 0.0f;

        // Mouse button (0=Left, 1=Right, 2=Middle, 3=X1, 4=X2)
        int mouseButton = 0;

        // Mouse wheel
        float wheelDelta = 0.0f;
    };

    class InputManager
    {
    public:
        static InputManager* GetInstance();

        virtual ~InputManager() = default;
        virtual void Initialize();
        virtual void Terminate();

        // Poll all pending events (platform-agnostic). Called once per frame.
        virtual std::vector<InputEvent> PollEvents() = 0;

        // Instantaneous queries (no cross-frame state)
        virtual InputVector2f GetMousePosition() const = 0;
        virtual bool IsMouseButtonDown(int button) const = 0;

    };

} // namespace uinput
