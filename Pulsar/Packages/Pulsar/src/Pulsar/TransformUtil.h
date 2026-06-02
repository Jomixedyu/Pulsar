#pragma once
#include "EngineMath.h"

namespace transutil
{
    using namespace pulsar::math;

    inline Vector3f Vector3Up()
    {
        return { 0,1,0 };
    }

    template<typename T>
    void NewTranslate(Matrix4<T>& mat, const Vector3<T>& translate)
    {
        mat.M[3][0] = translate.x;
        mat.M[3][1] = translate.y;
        mat.M[3][2] = translate.z;
    }

    template<typename T>
    void NewScale(Matrix4<T>& mat, const Vector3<T>& scale)
    {
        mat.M[0][0] = scale.x;
        mat.M[1][1] = scale.y;
        mat.M[2][2] = scale.z;
        mat.M[3][3] = 1;
    }

    template<typename T>
    void NewTRS(Matrix4<T>& mat, const Vector3<T>& translate, const Quaternion<T>& rotate, const Vector3<T>& scale)
    {
        new(&mat)Matrix4<T>(1);
        NewScale(mat, scale);
        mat = jmath::Rotate(rotate) * mat;
        NewTranslate(mat, translate);
    }
}