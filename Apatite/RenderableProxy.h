#pragma once
#include "Assembly.h"

namespace apatite
{
    class RenderableProxy
    {

    public:
        virtual void SingleDraw() = 0;
    };
}