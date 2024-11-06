#include "InputSystem/InputComponent.h"
#include <uinput/InputManager.h>

namespace pulsar
{

    void InputComponent::Bind(string_view name, SPtr<InputEventDelegate> callback)
    {
        m_callbacks.push_back(callback);

    }

    void InputComponent::OnTick(Ticker ticker)
    {

    }

    void InputComponent::BeginPlay()
    {
        base::BeginPlay();
        // uinput::InputManager::GetInstance()->AddKeyboardInput()
        //uinput::InputManager::AddKeyboardInput()

    }

    void InputComponent::EndPlay()
    {
        base::EndPlay();

    }

} // namespace pulsar