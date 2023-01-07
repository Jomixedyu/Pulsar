#pragma once
#include "Component.h"
#include <Apatite/Math.h>
#include <Apatite/Assets/RenderTexture.h>

namespace apatite
{
    enum class CameraMode
    {
        Perspective,
        Orthographic
    };

    class CameraComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::CameraComponent, Component);
    public:
        RenderTexture_sp render_target;
        void Render();
    public:
        float fov;
        float near;
        float far;
        Vector2f size;
        CameraMode cameraMode;
        LinearColorf backgroundColor;
    };
}