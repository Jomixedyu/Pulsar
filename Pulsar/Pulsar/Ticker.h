#pragma once
#include <Pulsar/ObjectBase.h>

namespace pulsar
{
    struct Ticker
    {
        float deltatime;
        uint32_t count;
    };

    class ITickable
    {
    public:
        virtual void OnTick(Ticker ticker) = 0;
    };
}