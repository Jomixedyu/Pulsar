#pragma once
#include "Math.h"

namespace transutil
{
    using namespace apatite::math;

    inline Vector3f Vector3Up()
    {
        return { 0,1,0 };
    }
    inline void Scale(Matrix4f* mat, const Vector3f& v3)
    {
        mat->M[0][0] *= v3.x;
        mat->M[1][1] *= v3.y;
        mat->M[2][2] *= v3.z;
    }
    inline void Rotate(Matrix4f* mat, const Quat4f& q)
    {
        float qxx(q.x * q.x);
        float qyy(q.y * q.y);
        float qzz(q.z * q.z);
        float qxz(q.x * q.z);
        float qxy(q.x * q.y);
        float qyz(q.y * q.z);
        float qwx(q.w * q.x);
        float qwy(q.w * q.y);
        float qwz(q.w * q.z);

        mat->M[0][0] = float(1) - float(2) * (qyy + qzz);
        mat->M[0][1] = float(2) * (qxy + qwz);
        mat->M[0][2] = float(2) * (qxz - qwy);

        mat->M[1][0] = float(2) * (qxy - qwz);
        mat->M[1][1] = float(1) - float(2) * (qxx + qzz);
        mat->M[1][2] = float(2) * (qyz + qwx);

        mat->M[2][0] = float(2) * (qxz + qwy);
        mat->M[2][1] = float(2) * (qyz - qwx);
        mat->M[2][2] = float(1) - float(2) * (qxx + qyy);
    }

    inline void Translate(Matrix4f* mat, const Vector3f& v3)
    {
        Vector4f v = mat->M[0] * v3.x + mat->M[1] * v3.y + mat->M[2] * v3.z + mat->M[3];
        mat->M[3] = v;
    }
}