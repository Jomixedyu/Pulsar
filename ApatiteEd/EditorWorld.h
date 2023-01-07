#pragma once

#include <Apatite/World.h>
#include "Assembly.h"

namespace apatiteed
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

