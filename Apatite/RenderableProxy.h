#pragma once
#include "ObjectBase.h"

namespace apatite
{
    class RenderableProxy
    {

    public:
        virtual void DrawSingle() = 0;
    };
}