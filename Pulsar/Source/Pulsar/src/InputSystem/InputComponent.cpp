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

        for (auto& action : m_actionMap->GetActions())
        {
        }

        uinput::InputManager::GetInstance()->AddKeyboardInput([this](uinput::KeyState, uinput::KeyCode c) {
            for (auto& action : m_actionMap->GetActions())
            {
                // for (auto& binding : *action->m_bindings)
                // {
                //     binding->m_modifier
                // }
                // action->m_bindings
            }
        });
    }

    void InputComponent::EndPlay()
    {
        base::EndPlay();

    }

} // namespace pulsar