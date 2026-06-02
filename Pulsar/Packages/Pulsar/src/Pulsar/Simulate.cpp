#include "Simulate.h"

namespace pulsar
{

    void SimulateManager::AddSimulate(ISimulate* simulate)
    {
        simulates.push_back(simulate);
    }
    void SimulateManager::RemoveSimulate(ISimulate* simulate)
    {
        std::erase(simulates, simulate);
    }
    void SimulateManager::SimulateTick(float dt)
    {
        for (auto& simulate : simulates)
        {
            simulate->SimulateTick(dt);
        }
    }
} // namespace pulsar