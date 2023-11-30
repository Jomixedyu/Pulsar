#include "Components/CameraComponent.h"
#include <Pulsar/Application.h>
#include <Pulsar/AppInstance.h>
#include <Pulsar/TransformUtil.h>
#include <Pulsar/Node.h>
#include "Scene.h"
#include "Logger.h"

namespace pulsar
{
    void CameraComponent::Render()
    {

    }

    static Matrix4f _GetViewMat(const Vector3f& eye, const Quat4f& rotation)
    {
        Matrix4f mat;
        transutil::Rotate(&mat, rotation);
        jmath::Transpose(&mat);

        Vector3f x(mat[0][0], mat[1][0], mat[2][0]);
        Vector3f y(mat[0][1], mat[1][1], mat[2][1]);
        Vector3f z(-mat[0][2], -mat[1][2], -mat[2][2]);

        Matrix4f result;

        result[0][0] = x.x;
        result[1][0] = x.y;
        result[2][0] = x.z;
        result[3][0] = -Vector3f::Dot(x, eye);
              
        result[0][1] = y.x;
        result[1][1] = y.y;
        result[2][1] = y.z;
        result[3][1] = -Vector3f::Dot(y, eye);
              
        result[0][2] = z.x;
        result[1][2] = z.y;
        result[2][2] = z.z;
        result[3][2] = -Vector3f::Dot(z, eye);
              
        result[0][3] = 0;
        result[1][3] = 0;
        result[2][3] = 0;
        result[3][3] = 1.0f;
        return result;
    }
    Matrix4f CameraComponent::GetViewMat()
    {
        //auto node = this->GetAttachedNode();
        //auto wpos = node->get_world_position();
        //Matrix4f mat = node->get_world_rotation().ToMatrix();
        //jmath::Transpose(&mat);
        //transutil::Translate(&mat, -wpos);
        //return mat;
        return {};
    }

    Matrix4f CameraComponent::GetProjectionMat()
    {
        const Vector2f& size = this->m_renderTarget ? this->m_renderTarget->GetSize2df() : this->size_;
        Matrix4f ret;
        if (this->cameraMode == CameraMode::Perspective)
        {
            ret = math::Perspective(
                math::Radians(this->fov),
                size.x / size.y,
                this->near,
                this->far);
        }
        else
        {
            ret = math::Ortho(
                0.0f,
                size.x,
                0.0f,
                size.y,
                this->near,
                this->far);
        }
        return ret;
    }
    Matrix4f CameraComponent::LookAtRH(Vector3f const& eye, Vector3f const& center, Vector3f const& up)
    {
        return {};
    }

    void CameraComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        if (info->GetName() == "m_backgroundColor")
        {
            UpdateRTBackgroundColor();
        }
    }
    void CameraComponent::SetBackgroundColor(const Color4f& value)
    {
        m_backgroundColor = value;
        UpdateRTBackgroundColor();
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

    void CameraComponent::SetRenderTarget(const RenderTexture_ref& value)
    {
        m_renderTarget = value;
        UpdateRT();
    }

    void CameraComponent::LookAt(const Vector3f& pos)
    {
        //transutil::LookAt(this->GetAttachedNode()->get_world_position(), pos, transutil::Vector3Up());
    }

    void CameraComponent::BeginComponent()
    {
        base::BeginComponent();
        GetAttachedNode()->GetRuntimeOwnerScene()->GetWorld()->GetCameraManager().AddCamera(THIS_REF);
    }

    void CameraComponent::EndComponent()
    {
        base::EndComponent();
        GetAttachedNode()->GetRuntimeOwnerScene()->GetWorld()->GetCameraManager().RemoveCamera(THIS_REF);
    }
}
