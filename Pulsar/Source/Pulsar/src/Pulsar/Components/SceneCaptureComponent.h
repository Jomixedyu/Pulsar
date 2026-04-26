#pragma once
#include "Pulsar/Assets/RenderTexture.h"
#include "Component.h"
#include "Pulsar/Rendering/RenderGraph/ScriptableCaptureRenderer.h"
#include <memory>

namespace pulsar
{
    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar,
        CaptureProjectionMode,
        Perspective,
        Orthographic);

    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar,
        RenderingPathMode,
        Forward,
        Deferred);

    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar,
        CaptureSourceMode,
        SceneColorRGB_AAlpha,
        FinalColorRGB,
        SceneDepthR);
}

CORELIB_DECL_BOXING(pulsar::CaptureProjectionMode, pulsar::BoxingCaptureProjectionMode);
CORELIB_DECL_BOXING(pulsar::RenderingPathMode, pulsar::BoxingRenderingPathMode);

namespace pulsar
{
    struct PerCaptureShaderParameter
    {
        Matrix4f MatrixV;
        Matrix4f InvMatrixV;
        Matrix4f MatrixP;
        Matrix4f InvMatrixP;
        Matrix4f MatrixVP;
        Matrix4f InvMatrixVP;
        Vector4f CamPosition;
        float CamNear;
        float CamFar;
        Vector2f Resolution;
        Vector4f _Padding1;
        Vector4f _Padding2;
        Matrix4f _Padding3;
    };
    static_assert(sizeof(PerCaptureShaderParameter) == 512);


    struct RenderCapturePassInfo
    {
        gfx::GFXFrameBufferObject_sp FrameBuffer;

    };

    class SceneCaptureComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SceneCaptureComponent, Component);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute);
    public:
        SceneCaptureComponent();
        void BeginComponent() override;
        void EndComponent() override;

        virtual void Render(array_list<RenderCapturePassInfo*>& passes) { }
        virtual bool CanRender() const { return true; }

    protected:
        CORELIB_REFL_DECL_FIELD(m_enabledCapture)
        bool m_enabledCapture = true;

    public:
        std::unique_ptr<ScriptableCaptureRenderer> m_captureRenderer;
    };




} // namespace pulsar