#pragma once
#include "Pulsar/Assets/RenderTexture.h"
#include "Component.h"

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
    struct TargetCBuffer
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
    static_assert(sizeof(TargetCBuffer) == 512);


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

        RCPtr<Material> GetPostprocess(size_t index) const { return m_postProcessMaterials->at(index); }
        void SetPostProcess(size_t index, const RCPtr<Material>& value) const;
        void AddPostProcess(const RCPtr<Material>& material);
        void RemovePostProcessAt(size_t index);
        void ClearPostProcess();
        size_t GetPostProcessCount() const { return m_postProcessMaterials->size(); }

        virtual void Render(array_list<RenderCapturePassInfo*>& passes) { }
        virtual bool CanRender() const { return true; }

    protected:
        CORELIB_REFL_DECL_FIELD(m_postProcessMaterials, new ListItemAttribute(cltypeof<Material>()));
        List_sp<RCPtr<Material>> m_postProcessMaterials;

        CORELIB_REFL_DECL_FIELD(m_enabledCapture)
        bool m_enabledCapture = true;

    public:
        RCPtr<RenderTexture> m_postprocessRtA;
        gfx::GFXDescriptorSet_sp m_postprocessDescA;
        RCPtr<RenderTexture> m_postprocessRtB;
        gfx::GFXDescriptorSet_sp m_postprocessDescB;
    };




} // namespace pulsar