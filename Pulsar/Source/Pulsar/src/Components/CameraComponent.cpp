#include "Components/CameraComponent.h"
#include "Logger.h"
#include "Scene.h"
#include <Pulsar/AppInstance.h>
#include <Pulsar/Application.h>
#include <Pulsar/Node.h>
#include <Pulsar/TransformUtil.h>

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

    CameraComponent::CameraComponent()
    {
        new_init_sptr(m_postProcessMaterials);
    }
    void CameraComponent::Render()
    {
    }

    Matrix4f CameraComponent::GetViewMat() const
    {
        return GetAttachedNode()->GetTransform()->GetWorldToLocalMatrix();
    }

    Matrix4f CameraComponent::GetProjectionMat() const
    {
        const Vector2f& size = this->m_renderTarget->GetSize2df();
        Matrix4f ret{1};
        if (this->m_projectionMode == CameraProjectionMode::Perspective)
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

    void CameraComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        auto name = info->GetName();
        if (name == NAMEOF(m_backgroundColor))
        {
            UpdateRTBackgroundColor();
        }
    }
    void CameraComponent::ResizeManagedRenderTexture(int width, int height)
    {
        if (!m_beginning)
            return;
        if (!m_managedRT)
            return;
        if (m_renderTarget)
        {
            DestroyObject(m_renderTarget);
        }

        auto rtname = GetAttachedNode()->GetName() + "_CamRT";
        m_renderTarget = RenderTexture::StaticCreate(index_string{rtname}, width, height);

        m_postprocessRtA = RenderTexture::StaticCreate(index_string{rtname}, width, height, 1, false);
        m_postprocessRtB = RenderTexture::StaticCreate(index_string{rtname}, width, height, 1, false);

        gfx::GFXDescriptorSetLayoutInfo ppDescLayouts[2]{
            {gfx::GFXDescriptorType::CombinedImageSampler,
             gfx::GFXShaderStageFlags::VertexFragment,
             0, 2},
            {gfx::GFXDescriptorType::CombinedImageSampler,
             gfx::GFXShaderStageFlags::VertexFragment,
             1, 2},
        };

        auto layout = Application::GetGfxApp()->CreateDescriptorSetLayout(ppDescLayouts, 2);
        m_postprocessDescA = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(layout);
        m_postprocessDescA->AddDescriptor("Color", 0)->SetTextureSampler2D(m_postprocessRtB->GetGfxRenderTarget0().get());
        m_postprocessDescA->Submit();
        m_postprocessDescB = Application::GetGfxApp()->GetDescriptorManager()->GetDescriptorSet(layout);
        m_postprocessDescB->AddDescriptor("Color", 0)->SetTextureSampler2D(m_postprocessRtA->GetGfxRenderTarget0().get());
        m_postprocessDescB->Submit();

        UpdateRT();
        BeginRT();
    }
    void CameraComponent::OnTick(Ticker ticker)
    {
        base::OnTick(ticker);
        m_debugViewMat = GetViewMat();
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
    void CameraComponent::AddPostProcess(Material_ref material)
    {
        m_postProcessMaterials->push_back(material);
    }

    void CameraComponent::SetRenderTexture(const RenderTexture_ref& value, bool managed)
    {
        if (value)
        {
            if (m_managedRT)
            {
                DestroyObject(m_renderTarget);
                m_managedRT = false;
            }
            m_renderTarget = value;
            m_managedRT = managed;
        }
        else // null
        {
            if (!m_managedRT)
            {
                m_renderTarget = value;
                m_managedRT = managed;
            }
        }

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

        if (!m_renderTarget)
        {
            m_managedRT = true;
            ResizeManagedRenderTexture(1, 1);
        }

        GetAttachedNode()->GetRuntimeOwnerScene()->GetWorld()->GetCameraManager().AddCamera(THIS_REF);

        if (_CameraDescriptorLayout.expired())
        {
            gfx::GFXDescriptorSetLayoutInfo info{
                gfx::GFXDescriptorType::ConstantBuffer,
                gfx::GFXShaderStageFlags::VertexFragment,
                0, kRenderingDescriptorSpace_Camera};
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
        if (m_managedRT && m_renderTarget)
        {
            DestroyObject(m_renderTarget);
        }
        m_camDescriptorLayout.reset();
        m_cameraDescriptorSet.reset();
        m_cameraDataBuffer.reset();
    }
} // namespace pulsar
