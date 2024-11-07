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
} // namespace pulsar