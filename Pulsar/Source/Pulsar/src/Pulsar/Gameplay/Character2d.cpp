#include "Gameplay/Character2d.h"
#include "Input.h"
#include "Logger.h"
#include "Node.h"

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
        if (Input::GetKey(KeyCode::A))
            h -= 1;
        if (Input::GetKey(KeyCode::D))
            h += 1;
        GetTransform()->Translate({ h * ticker.deltatime, 0, 0});
    }
}
