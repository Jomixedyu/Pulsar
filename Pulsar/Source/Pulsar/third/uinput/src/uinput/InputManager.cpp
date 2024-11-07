#include "InputManager.h"
#include "InputManagerSDL.h"

namespace uinput
{

    InputManager* InputManager::GetInstance()
    {
        static InputManagerSDL instance{};
        return &instance;
    }
    void InputManager::Initialize()
    {
    }
    void InputManager::ProcessEvents()
    {
    }

    int InputManager::AddKeyboardInput(const std::function<void(KeyState, KeyCode)>& func)
    {
        static int id = 0;

        ++id;

        m_keyboard[id] = func;
        return id;
    }

    void InputManager::RemoveKeyboardInput(int key)
    {
        m_keyboard.erase(key);
    }

    void InputManager::BroadcastKeyboard(KeyState mode, KeyCode code)
    {
        for (auto& [i, func] : m_keyboard)
        {
            func(mode, code);
        }
    }

} // namespace uinput