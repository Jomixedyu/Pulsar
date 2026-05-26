#include "Gameplay/Character2d.h"
#include "Components/InputComponent.h"
#include "Logger.h"
#include "Node.h"
#include "World.h"

namespace pulsar
{
    void Character2d::BeginPlay()
    {
        base::BeginPlay();
    }

    void Character2d::EndPlay()
    {
        base::EndPlay();
    }

    void Character2d::OnTick(Ticker ticker)
    {
        base::OnTick(ticker);
        float h = 0;
        if (auto input = GetNode()->GetComponent<InputComponent>())
        {
            h = input->GetAxis("Horizontal");
        }
        GetTransform()->Translate({ h * ticker.deltatime, 0, 0});
    }
}
