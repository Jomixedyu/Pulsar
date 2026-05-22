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
    void InputManager::Terminate()
    {
    }
    std::vector<InputEvent> InputManager::PollEvents()
    {
        return {};
    }

} // namespace uinput