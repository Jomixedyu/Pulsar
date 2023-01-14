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
        Matrix4f GetViewMat();
        Matrix4f GetProjectionMat();
        static Matrix4f LookAtRH(
            Vector3f const& eye,
            Vector3f const& center,
            Vector3f const& up);

    public:
        float fov;
        float near;
        float far;
        Vector2f size_;
        CameraMode cameraMode;
        LinearColorf backgroundColor;
    };
    CORELIB_DECL_SHORTSPTR(CameraComponent);
}