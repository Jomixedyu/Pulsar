#include "Input.h"

namespace pulsar
{
    void InputContext::BeginFrame()
    {
        previousKeys = currentKeys;
        previousMouseButtons = currentMouseButtons;
        prevMousePosition = mousePosition;
        prevMouseWheel = mouseWheel;
    }

    void InputContext::ProcessEvent(const uinput::InputEvent& e)
    {
        switch (e.type)
        {
        case uinput::InputEvent::KeyDown:
            currentKeys.insert(e.keyCode);
            break;
        case uinput::InputEvent::KeyUp:
            currentKeys.erase(e.keyCode);
            break;
        case uinput::InputEvent::MouseMove:
            mousePosition = Vector2f(e.mouseX, e.mouseY);
            break;
        case uinput::InputEvent::MouseButtonDown:
            currentMouseButtons.insert(e.mouseButton);
            break;
        case uinput::InputEvent::MouseButtonUp:
            currentMouseButtons.erase(e.mouseButton);
            break;
        case uinput::InputEvent::MouseWheel:
            mouseWheel += e.wheelDelta;
            break;
        }
    }

    // --- Query implementations ---
    bool InputContext::GetKey(KeyCode key) const
    {
        return currentKeys.contains(key);
    }

    bool InputContext::GetKeyDown(KeyCode key) const
    {
        return currentKeys.contains(key) && !previousKeys.contains(key);
    }

    bool InputContext::GetKeyUp(KeyCode key) const
    {
        return !currentKeys.contains(key) && previousKeys.contains(key);
    }

    Vector2f InputContext::GetMousePosition() const
    {
        return mousePosition;
    }

    Vector2f InputContext::GetMouseDelta() const
    {
        return mousePosition - prevMousePosition;
    }

    bool InputContext::GetMouseButton(int button) const
    {
        return currentMouseButtons.contains(button);
    }

    bool InputContext::GetMouseButtonDown(int button) const
    {
        return currentMouseButtons.contains(button) && !previousMouseButtons.contains(button);
    }

    bool InputContext::GetMouseButtonUp(int button) const
    {
        return !currentMouseButtons.contains(button) && previousMouseButtons.contains(button);
    }

    float InputContext::GetMouseScrollWheel() const
    {
        return mouseWheel;
    }

    float InputContext::GetMouseScrollWheelDelta() const
    {
        return mouseWheel - prevMouseWheel;
    }
}
