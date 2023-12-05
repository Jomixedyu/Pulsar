#include "Components/CameraComponent.h"
#include <Pulsar/Application.h>
#include <Pulsar/AppInstance.h>
#include <Pulsar/TransformUtil.h>
#include <Pulsar/Node.h>
#include "Scene.h"
#include "Logger.h"

namespace pulsar
{
    struct CBuffer_Target
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

    static_assert(sizeof(CBuffer_Target) == 512);

    void CameraComponent::Render()
    {

    }

    Matrix4f CameraComponent::GetViewMat() const
    {
        return jmath::Inverse(GetAttachedNode()->GetTransform()->GetChildLocalToWorldMatrix());
    }

    Matrix4f CameraComponent::GetProjectionMat() const
    {
        const Vector2f& size = this->m_renderTarget->GetSize2df();
        Matrix4f ret{1};
        if (this->m_projectionMode == CameraProjectionMode::Perspective)
        {
            math::Perspective(ret,
                math::Radians(this->m_fov),
                size.x / size.y,
                this->m_near,
                this->m_far);
        }
        else
        {
            math::Ortho(ret,
                0.0f,
                size.x,
                0.0f,
                size.y,
                this->m_near,
                this->m_far);
        }
        return ret;
    }

    void CameraComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        if (info->GetName() == "m_backgroundColor")
        {
            UpdateRTBackgroundColor();
        }
    }
    void CameraComponent::OnTick(Ticker ticker)
    {
        base::OnTick(ticker);
        debug_view_mat = GetViewMat();
        UpdateCBuffer();
    }
    void CameraComponent::SetBackgroundColor(const Color4f& value)
    {
        m_backgroundColor = value;
        UpdateRTBackgroundColor();
    }
    void CameraComponent::SetProjectionMode(CameraProjectionMode mode)
    {
        m_projectionMode = mode;
    }

    void CameraComponent::UpdateRTBackgroundColor()
    {
        if (!m_renderTarget)
        {
            return;
        }
        auto rt0 = m_renderTarget->GetGfxRenderTarget0().get();
        rt0->ClearColor[0] = m_backgroundColor.r;
        rt0->ClearColor[1] = m_backgroundColor.g;
        rt0->ClearColor[2] = m_backgroundColor.b;
        rt0->ClearColor[3] = m_backgroundColor.a;
    }
    void CameraComponent::UpdateRT()
    {
        UpdateRTBackgroundColor();
    }
    void CameraComponent::UpdateCBuffer()
    {
        CBuffer_Target target{};
        target.MatrixV = GetViewMat();
        target.MatrixP = GetProjectionMat();
        target.MatrixVP = target.MatrixP * target.MatrixV;

        target.InvMatrixV = jmath::Inverse(target.MatrixV);
        target.InvMatrixP = jmath::Inverse(target.MatrixP);
        target.InvMatrixVP = jmath::Inverse(target.MatrixVP);
        target.CamPosition = GetAttachedNode()->GetTransform()->GetWorldPosition();
        target.CamNear = m_near;
        target.CamFar = m_far;
        target.Resolution = m_renderTarget->GetSize2df();
        m_cameraDataBuffer->Fill(&target);
    }

    void CameraComponent::SetRenderTarget(const RenderTexture_ref& value)
    {
        m_renderTarget = value;
        UpdateRT();
        BeginRT();
    }
    void CameraComponent::BeginRT()
    {
        if (m_beginning && m_renderTarget)
        {
            auto& refData = m_renderTarget->GetGfxFrameBufferObject()->RefData;
            refData.clear();
            refData.push_back(m_cameraDescriptorSet);
        }
    }

    static gfx::GFXDescriptorSetLayout_wp _CameraDescriptorLayout;

    void CameraComponent::BeginComponent()
    {
        base::BeginComponent();
        GetAttachedNode()->GetRuntimeOwnerScene()->GetWorld()->GetCameraManager().AddCamera(THIS_REF);

        if(_CameraDescriptorLayout.expired())
        {
            gfx::GFXDescriptorSetLayoutInfo info{
                gfx::GFXDescriptorType::ConstantBuffer,
                gfx::GFXShaderStageFlags::VertexFragment,
                0, kRenderingDescriptorSpace_Camera
            };
            m_camDescriptorLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(&info, 1);
            _CameraDescriptorLayout = m_camDescriptorLayout;
        }
        else
        {
            m_camDescriptorLayout = _CameraDescriptorLayout.lock();
        }

        m_cameraDataBuffer = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::ConstantBuffer, sizeof(CBuffer_Target));
        m_cameraDescriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_camDescriptorLayout);
        m_cameraDescriptorSet->AddDescriptor("Target", 0)->SetConstantBuffer(m_cameraDataBuffer.get());
        m_cameraDescriptorSet->Submit();

        BeginRT();
    }

    void CameraComponent::EndComponent()
    {
        base::EndComponent();
        GetAttachedNode()->GetRuntimeOwnerScene()->GetWorld()->GetCameraManager().RemoveCamera(THIS_REF);
        m_camDescriptorLayout.reset();
        m_cameraDescriptorSet.reset();
        m_cameraDataBuffer.reset();
    }
}
