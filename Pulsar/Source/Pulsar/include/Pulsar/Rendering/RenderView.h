#pragma once
#include "CoreLib.Math/Jmath.h"

namespace pulsar
{
    class RenderView;

    class IRenderView
    {
    public:
        virtual RenderView* GetRenderView() = 0;
    };

    class RenderView
    {
    public:
        jmath::Matrix4f Transform;
    };
} // namespace pulsar