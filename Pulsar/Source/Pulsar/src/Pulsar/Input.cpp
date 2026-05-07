#include "Input.h"
#include <uinput/InputManager.h>

namespace pulsar
{
    int Input::s_callbackHandle = -1;
    std::set<KeyCode> Input::s_currentKeys;
    std::set<KeyCode> Input::s_previousKeys;

    void Input::Initialize()
    {
        s_callbackHandle = uinput::InputManager::GetInstance()->AddKeyboardInput([](uinput::KeyState state, KeyCode c) {
            if (state == uinput::KeyState::Down)
            {
                s_currentKeys.insert(c);
            }
            else if (state == uinput::KeyState::Up)
            {
                s_currentKeys.erase(c);
            }
        });
    }

    void Input::Shutdown()
    {
        uinput::InputManager::GetInstance()->RemoveKeyboardInput(s_callbackHandle);
    }

    void Input::Update()
    {
        s_previousKeys = s_currentKeys;
    }

    bool Input::GetKey(KeyCode key)
    {
        return s_currentKeys.contains(key);
    }

    bool Input::GetKeyDown(KeyCode key)
    {
        return s_currentKeys.contains(key) && !s_previousKeys.contains(key);
    }

    bool Input::GetKeyUp(KeyCode key)
    {
        return !s_currentKeys.contains(key) && s_previousKeys.contains(key);
    }
}
