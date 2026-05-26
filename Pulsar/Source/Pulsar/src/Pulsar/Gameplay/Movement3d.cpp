#include "Gameplay/Movement3d.h"
#include "Components/InputComponent.h"
#include "Logger.h"
#include "Node.h"
#include "World.h"

namespace pulsar
{
    void Movement3d::BeginPlay()
    {
        base::BeginPlay();
    }

    void Movement3d::EndPlay()
    {
        base::EndPlay();
    }

    void Movement3d::OnTick(Ticker ticker)
    {
        base::OnTick(ticker);

        float h = 0.0f;
        float v = 0.0f;
        if (auto input = GetNode()->GetComponent<InputComponent>())
        {
            h = input->GetAxis("Horizontal");
            v = input->GetAxis("Vertical");
        }

        Vector3f delta{
            h * m_speed * ticker.deltatime,
            0.0f,
            v * m_speed * ticker.deltatime
        };
        GetTransform()->Translate(delta);
    }
}
