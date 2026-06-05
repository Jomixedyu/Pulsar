#pragma once
#include "Assembly.h"

namespace pulsar
{
    class ISimulate
    {
    public:
        virtual ~ISimulate() = default;
        virtual void BeginSimulate() = 0;
        virtual void EndSimulate() = 0;
        virtual void SimulateTick(float dt) {}
    };

    class SimulateManager final
    {
    public:

        void AddSimulate(ISimulate* simulate);
        void RemoveSimulate(ISimulate* simulate);
        void SimulateTick(float dt);

        const array_list<ISimulate*>& GetSimulates() const { return simulates; }
    protected:
        array_list<ISimulate*> simulates;
    };
}