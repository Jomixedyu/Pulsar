#include "Components/SceneCapture2DComponent.h"

#include "Application.h"
#include "Node.h"

namespace pulsar
{
    SceneCapture2DComponent::SceneCapture2DComponent()
        : m_debugViewMat({})
    {
        m_renderingPath = RenderingPathMode::Deferred;
    }

    static gfx::GFXDescriptorSetLayout_wp _CameraDescriptorLayout;

    constexpr uint32_t kRenderingDescriptorSpace_Camera = 0;

    void SceneCapture2DComponent::BeginComponent()
    {
        base::BeginComponent();

        if (_CameraDescriptorLayout.expired())
        {
            gfx::GFXDescriptorSetLayoutInfo info{
                gfx::GFXDescriptorType::ConstantBuffer,
                gfx::GFXShaderStageFlags::VertexFragment,
                0,
                kRenderingDescriptorSpace_Camera};
            m_camDescriptorLayout = Application::GetGfxApp()->CreateDescriptorSetLayout(&info, 1);
            _CameraDescriptorLayout = m_camDescriptorLayout;
        }
        else
        {
            m_camDescriptorLayout = _CameraDescriptorLayout.lock();
        }

        m_cameraDataBuffer = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::ConstantBuffer, sizeof(TargetCBuffer));
        m_cameraDescriptorSet = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(m_camDescriptorLayout);
        m_cameraDescriptorSet->AddDescriptor("Target", 0)->SetConstantBuffer(m_cameraDataBuffer.get());
        m_cameraDescriptorSet->Submit();
    }
    void SceneCapture2DComponent::EndComponent()
    {
        SceneCaptureComponent::EndComponent();
    }
    Matrix4f SceneCapture2DComponent::GetViewMat() const
    {
        return GetNode()->GetTransform()->GetWorldToLocalMatrix();
    }

    Matrix4f SceneCapture2DComponent::GetProjectionMat() const
    {
        const Vector2f& size = this->m_renderTarget->GetSize2df();
        Matrix4f ret{1};
        if (this->m_projectionMode == CaptureProjectionMode::Perspective)
        {
            math::Perspective_LHZO(ret,
                                   math::Radians(this->m_fov),
                                   size.x / size.y,
                                   this->m_near,
                                   this->m_far);
        }
        else
        {
            math::Ortho_LHZO(ret,
                             -size.x / 2 * m_orthoSize / 100,
                             size.x / 2 * m_orthoSize / 100,
                             -size.y / 2 * m_orthoSize / 100,
                             size.y / 2 * m_orthoSize / 100,
                             this->m_near,
                             this->m_far);
        }
        return ret;
    }

    Matrix4f SceneCapture2DComponent::GetInvViewProjectionMat() const
    {
        return Inverse(GetViewMat()) * Inverse(GetProjectionMat());
    }

    void SceneCapture2DComponent::SetFOV(float value)
    {
        m_fov = value;
        UpdateCBuffer();
    }
    void SceneCapture2DComponent::SetNear(float value)
    {
        m_near = value;
        UpdateCBuffer();
    }
    void SceneCapture2DComponent::SetFar(float value)
    {
        m_far = value;
        UpdateCBuffer();
    }
    void SceneCapture2DComponent::SetBackgroundColor(const Color4f& value)
    {
        m_backgroundColor = value;
        UpdateRTBackgroundColor();
        UpdateCBuffer();
    }
    void SceneCapture2DComponent::SetProjectionMode(CaptureProjectionMode mode)
    {
        m_projectionMode = mode;
        UpdateCBuffer();
    }
    void SceneCapture2DComponent::SetOrthoSize(float value)
    {
        m_orthoSize = value;
        UpdateCBuffer();
    }

    void SceneCapture2DComponent::OnTransformChanged()
    {
        base::OnTransformChanged();
        UpdateCBuffer();
    }


    void SceneCapture2DComponent::UpdateRTBackgroundColor()
    {
        if (!m_renderTarget)
        {
            return;
        }
        auto rt0 = m_renderTarget->GetGfxRenderTarget0();
        auto& color = rt0->GetTexture()->TargetClearColor;

        color[0] = m_backgroundColor.r;
        color[1] = m_backgroundColor.g;
        color[2] = m_backgroundColor.b;
        color[3] = m_backgroundColor.a;
    }

    void SceneCapture2DComponent::UpdateCBuffer()
    {
        m_debugViewMat = GetViewMat();

        TargetCBuffer target{};
        target.MatrixV = GetViewMat();
        target.MatrixP = GetProjectionMat();
        target.MatrixVP = target.MatrixP * target.MatrixV;

        target.InvMatrixV = jmath::Inverse(target.MatrixV);
        target.InvMatrixP = jmath::Inverse(target.MatrixP);
        target.InvMatrixVP = jmath::Inverse(target.MatrixVP);
        target.CamPosition = GetNode()->GetTransform()->GetWorldPosition();
        target.CamNear = m_near;
        target.CamFar = m_far;
        target.Resolution = m_renderTarget->GetSize2df();
        m_cameraDataBuffer->Fill(&target);
    }
} // namespace pulsar