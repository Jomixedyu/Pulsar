#include "InputSystem/InputComponent.h"
#include <uinput/InputManager.h>

namespace pulsar
{

    void InputComponent::Bind(string_view name, SPtr<InputEventDelegate> callback)
    {
        // m_eventMap.emplace(string{name}, callback);
    }

    void InputComponent::OnTick(Ticker ticker)
    {
        if (!m_actionMap)
        {
            return;
        }
        for (int i = 0; i < m_actionMap->GetActionCount(); ++i)
        {
            auto action = m_actionMap->GetActionAt(i);
            for (auto& binding : action->m_bindings)
            {
                if (auto keyboard = sptr_cast<InputActionKeyboardBinding>(binding))
                {
                    // if (m_state.m_downKey.contains(keyboard->m_code) && )
                    // {
                    //
                    // }
                    // keyboardMap[keyboard] = m_actionMap->GetActionNameAt(i);
                }
            }
        }
    }

    void InputComponent::BeginPlay()
    {
        base::BeginPlay();
        // uinput::InputManager::GetInstance()->AddKeyboardInput()
        //uinput::InputManager::AddKeyboardInput()
        m_keyboardCallbackHandle = uinput::InputManager::GetInstance()->AddKeyboardInput([this](uinput::KeyState state, KeyCode c) {
            if (state == uinput::KeyState::Down)
            {
                m_keyboard.m_downKey.insert(c);
            }
            else if (state == uinput::KeyState::Up)
            {
                m_keyboard.m_downKey.erase(c);
            }
        });
    }

    void InputComponent::EndPlay()
    {
        base::EndPlay();
        uinput::InputManager::GetInstance()->RemoveKeyboardInput(m_keyboardCallbackHandle);
    }

    void InputComponent::RegisterInputMap()
    {
        hash_map<SPtr<InputActionKeyboardBinding>, string> keyboardMap;
        for (int i = 0; i < m_actionMap->GetActionCount(); ++i)
        {
            auto action = m_actionMap->GetActionAt(i);
            for (auto& binding : action->m_bindings)
            {
                if (auto keyboard = sptr_cast<InputActionKeyboardBinding>(binding))
                {
                    keyboardMap[keyboard] = m_actionMap->GetActionNameAt(i);
                }
            }
        }


    }

} // namespace pulsar