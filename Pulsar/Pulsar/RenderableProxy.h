#pragma once
#include "ObjectBase.h"

namespace pulsar
{
    class RenderableProxy
    {

    public:
        virtual void DrawSingle() = 0;
    };
}