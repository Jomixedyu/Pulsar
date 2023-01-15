#include "CameraComponent.h"
#include <Apatite/Private/RenderInterface.h>
#include <ApatiteEd/CoordinateGrid.h>
#include <Apatite/Application.h>
#include <Apatite/AppInstance.h>
#include <Apatite/TransformUtil.h>

namespace apatite
{
    void CameraComponent::Render()
    {
        if (IsValid(this->render_target))
        {
            assert(this->render_target->GetIsBindGPU());

            RenderTextureScope rt{ this->render_target };

            auto [r, g, b, a] = this->backgroundColor;
            detail::RenderInterface::Clear(r, g, b, a);
            //draw ...
            apatiteed::CoordinateGrid::Render();

        }
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
        auto node = this->get_node();
        return math::LookAt(this->get_node()->get_world_position(), { 0,0,0 }, transutil::Vector3Up());
        //return _GetViewMat(node->get_world_position(), node->get_world_rotation());
    }

    Matrix4f CameraComponent::GetProjectionMat()
    {
        const Vector2f& size = this->render_target ? this->render_target->GetSize2df() : this->size_;
        Matrix4f ret;
        if (this->cameraMode == CameraMode::Perspective)
        {
            ret = math::Perspective(
                math::Radians(this->fov),
                size.x / size.y,
                this->near,
                this->far
            );
        }
        else
        {
            ret = math::Ortho(
                0.0f,
                size.x,
                0.0f,
                size.y,
                this->near,
                this->far
            );
        }
        return ret;
    }

}
