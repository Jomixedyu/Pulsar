#include "Gameplay/Character2d.h"

#include "AppInstance.h"
#include "Application.h"
#include "InputSystem/InputComponent.h"
#include "Logger.h"
#include "Node.h"

namespace pulsar
{

    void Character2d::BeginPlay()
    {
        base::BeginPlay();

        m_inputComponent = GetNode()->GetComponent<InputComponent>();

        if (!m_inputComponent)
        {
            m_inputComponent = GetNode()->AddComponent<InputComponent>();
            return;
        }

        auto callback = InputComponent::InputEventDelegate::FromWeakMember(self_weak(), &ThisClass::OnInput);

        m_inputComponent->Bind("Horizontal", callback);

    }

    void Character2d::EndPlay()
    {
        base::EndPlay();

    }

    void Character2d::OnTick(Ticker ticker)
    {
        base::OnTick(ticker);
        if (m_inputComponent->m_keyboard.IsKeyDown(KeyCode::A))
        {
            GetTransform()->Translate({ -1.f * ticker.deltatime, 0, 0});
        }
        if (m_inputComponent->m_keyboard.IsKeyDown(KeyCode::D))
        {
            GetTransform()->Translate({ 1.f * ticker.deltatime, 0, 0});
        }
    }

    void Character2d::OnInput(SPtr<InputContext> ctx)
    {
        auto vec = ctx->GetVector2();
        auto newVec = Vector3f(vec.x, vec.y, 0.0f);
        GetTransform()->Translate(newVec);

    }
} // namespace pulsar