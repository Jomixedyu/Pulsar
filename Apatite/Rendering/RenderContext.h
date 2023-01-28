#pragma once
#include <Apatite/Components/CameraComponent.h>

namespace apatite
{
    class RenderContext
    {
    public:
        static void PushCamera(CameraComponent_sp cam);
        static void PopCamera();
        static CameraComponent_sp GetCurrentCamera();
    };
}