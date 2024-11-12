#pragma once
#include "InputDevice.h"
#include <memory>
#include <vector>

#include <functional>
#include "InputKeyCode.h"

namespace uinput
{
    enum class InputValueType
    {
        Bool,
        Float,
        Float2,
        Float3,
    };


    enum class KeyState
    {
        None,
        Down,
        Up
    };

    class InputManager
    {
    public:

        static InputManager* GetInstance();
    public:
        virtual ~InputManager() = default;
        virtual void Initialize();
        virtual void Terminate();
        virtual void ProcessEvents();

        virtual int AddKeyboardInput(const std::function<void(KeyState, KeyCode)>& func);
        virtual void RemoveKeyboardInput(int key);

        void BroadcastKeyboard(KeyState mode, KeyCode code);

    private:
        std::unordered_map<int, std::function<void(KeyState, KeyCode)>> m_keyboard;

        std::vector<std::unique_ptr<InputDevice>> m_devices;
    };

} // namespace uinput