#pragma once
#include "SceneCaptureComponent.h"

namespace pulsar
{

    class SceneCapture2DComponent : public SceneCaptureComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SceneCapture2DComponent, SceneCaptureComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Capture"));
    public:
        SceneCapture2DComponent();

        void BeginComponent() override;
        void EndComponent() override;

        Matrix4f GetViewMat() const;
        Matrix4f GetProjectionMat() const;
        Matrix4f GetInvViewProjectionMat() const;

        float GetFOV() const { return m_fov; }
        void  SetFOV(float value);
        float GetNear() const { return m_near; }
        void  SetNear(float value);
        float GetFar() const { return m_far; }
        void  SetFar(float value);
        Color4f  GetBackgroundColor() const { return m_backgroundColor; }
        void     SetBackgroundColor(const Color4f& value);
        CaptureProjectionMode  GetProjectionMode() const { return m_projectionMode; }
        void                   SetProjectionMode(CaptureProjectionMode mode);

        float GetOrthoSize() const { return m_orthoSize; }
        void SetOrthoSize(float value);

        void OnTransformChanged() override;

    protected:
        void UpdateRTBackgroundColor();
        void UpdateCBuffer();

    protected:
        gfx::GFXDescriptorSetLayout_sp m_camDescriptorLayout;
        gfx::GFXDescriptorSet_sp m_cameraDescriptorSet;
        gfx::GFXBuffer_sp m_cameraDataBuffer;

        CORELIB_REFL_DECL_FIELD(m_fov);
        float m_fov{};

        CORELIB_REFL_DECL_FIELD(m_near);
        float m_near{};

        CORELIB_REFL_DECL_FIELD(m_far);
        float m_far{};

        CORELIB_REFL_DECL_FIELD(m_projectionMode);
        CaptureProjectionMode m_projectionMode{};

        CORELIB_REFL_DECL_FIELD(m_backgroundColor);
        Color4f m_backgroundColor{};

        CORELIB_REFL_DECL_FIELD(m_renderTarget);
        RCPtr<RenderTexture> m_renderTarget;

        CORELIB_REFL_DECL_FIELD(m_orthoSize);
        float m_orthoSize = 1;

        CORELIB_REFL_DECL_FIELD(m_renderingPath);
        RenderingPathMode m_renderingPath;

        bool m_managedRT{false};
        #ifdef WITH_EDITOR
        CORELIB_REFL_DECL_FIELD(m_debugViewMat, new DebugPropertyAttribute, new ReadOnlyPropertyAttribute);
        Matrix4f m_debugViewMat;
        #endif

    protected:

        RenderTargetShaderParameter m_targetBuffer{};

        RCPtr<RenderTexture> m_sceneColor;
    };
} // namespace pulsar
