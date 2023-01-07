#pragma once
#include <Apatite/ObjectBase.h>

namespace apatite
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