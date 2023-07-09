#pragma once
#include <Pulsar/Components/CameraComponent.h>

namespace pulsar
{
    class RenderContext
    {
    public:
        static void PushCamera(CameraComponent_sp cam);
        static void PopCamera();
        static CameraComponent_sp GetCurrentCamera();
    };
}