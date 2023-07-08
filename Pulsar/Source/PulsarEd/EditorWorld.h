#pragma once

#include <Pulsar/World.h>
#include "Assembly.h"

namespace pulsared
{
    class EditorWorld : public World
    {
    public:
        virtual void Tick(float dt) override;

    protected:
        virtual void OnWorldBegin() override;
        virtual void OnWorldEnd() override;
    };
}

