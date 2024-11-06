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

        inputComponent = GetNode()->GetComponent<InputComponent>();

        if (!inputComponent)
        {
            return;
        }

        auto callback = InputComponent::InputEventDelegate::FromWeakMember(self_weak(), &ThisClass::OnInput);

        inputComponent->Bind("Horizontal", callback);

    }

    void Character2d::EndPlay()
    {
        base::EndPlay();

    }

    void Character2d::OnTick(Ticker ticker)
    {
        base::OnTick(ticker);
    }
    void Character2d::OnInput(SPtr<InputContext> ctx)
    {

    }
} // namespace pulsar