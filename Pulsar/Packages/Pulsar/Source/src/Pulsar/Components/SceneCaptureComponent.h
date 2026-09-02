#pragma once
#include <Pulsar/IconsForkAwesome.h>
#include "Pulsar/Assets/RenderTexture.h"
#include "Pulsar/Assets/ViewPipelineSettings.h"
#include <Pulsar/Rendering/SceneView.h>
#include "RenderComponent.h"
#include <memory>

namespace pulsar
{
    class SceneView;

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

    class SceneCaptureComponent : public RenderComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SceneCaptureComponent, RenderComponent);
        CORELIB_CLASS_ATTR(new AbstractComponentAttribute, new ComponentIconAttribute(ICON_FK_TELEVISION));
    public:
        SceneCaptureComponent();
        void SetViewPipelineSettings(const RCPtr<ViewPipelineSettings>& value);
        void MarkPostProcessDirty();
        void BeginComponent() override;
        void EndComponent() override;
        void PostEditChange(FieldInfo* info) override;

        virtual void Render(array_list<RenderCapturePassInfo*>& passes) { }
        virtual bool CanRender() const { return true; }

        // Fills the render-side parameter snapshot for this view. Overridden by
        // concrete capture types (2D fills view/proj/RT; camera adds gizmo flag).
        // Returns false if this capture cannot produce a valid view this frame.
        virtual bool ExtractViewData(SceneViewData& outData) { return false; }


    protected:
        // Unified proxy hooks: the proxy is a SceneView owned by the render thread.
        SPtr<rendering::RenderProxy> CreateRenderProxy() override;
        void ResolveRenderStateDirty() override;
        void GetSubscribeObserverHandles(array_list<ObjectHandle>& out) override;
        void OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg) override;

        // Cached typed view of m_proxy (the SceneView this component owns). Kept as a
        // shared ref like mesh renderers' m_renderObject; the render scene owns the
        // canonical instance. Used to address this view's snapshot in UpdateSceneView.
        SPtr<SceneView> m_sceneView;
        SceneViewData m_sceneViewData;

        bool m_renderDirtyTransform = true;
        bool m_renderDirtyCamera = true;
        bool m_renderDirtyRenderTarget = true;
        bool m_renderDirtyRenderFeature = true;
        bool m_renderDirtyPostProcess = true;

        CORELIB_REFL_DECL_FIELD(m_viewPipelineSettings)
        RCPtr<ViewPipelineSettings> m_viewPipelineSettings;


        CORELIB_REFL_DECL_FIELD(m_enabledCapture)
        bool m_enabledCapture = true;
    };




} // namespace pulsar
