#pragma once
#include <CoreLib.Math/Math.h>
#include <CoreLib.Serialization/DataSerializer.h>

namespace apatite::math
{
    using namespace jxcorlib;
    using namespace jxcorlib::math;

    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, Vector2f& vec);
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, Vector3f& vec);
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, Vector4f& vec);
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, Quat4f& q);
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, Color8b4& c);
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, LinearColorf& c);

    inline Matrix4f Perspective(const float& fovy, const float& aspect, const float& zNear, const float& zFar)
    {
        float const tanHalfFovy = tan(fovy / 2);
        Matrix4f result;
        result[0][0] = 1 / (aspect * tanHalfFovy);
        result[1][1] = 1 / (tanHalfFovy);
        result[2][3] = -1;

        result[2][2] = -(zFar + zNear) / (zFar - zNear);
        result[3][2] = -(2 * zFar * zNear) / (zFar - zNear);
        return result;
    }

    inline Matrix4f Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
    {
        Matrix4f Result = Matrix4f::StaticScalar();
        Result[0][0] = 2 / (right - left);
        Result[1][1] = 2 / (top - bottom);
        Result[3][0] = -(right + left) / (right - left);
        Result[3][1] = -(top + bottom) / (top - bottom);

        Result[2][2] = -2 / (zFar - zNear);
        Result[3][2] = -(zFar + zNear) / (zFar - zNear);

        return Result;
    }
    inline Matrix4f LookAt(const Vector3f& eye, const Vector3f& center, const Vector3f& up)
    {
        Vector3f f(Vector3f::Normalize(center - eye));
        
        Vector3f s(Vector3f::Normalize(Vector3f::Cross(f, up)));
        Vector3f u(Vector3f::Cross(s, f));

        Matrix4f Result = Matrix4f::StaticScalar();
        Result[0][0] = s.x;
        Result[1][0] = s.y;
        Result[2][0] = s.z;
        Result[0][1] = u.x;
        Result[1][1] = u.y;
        Result[2][1] = u.z;
        Result[0][2] = -f.x;
        Result[1][2] = -f.y;
        Result[2][2] = -f.z;
        Result[3][0] = -Vector3f::Dot(s, eye);
        Result[3][1] = -Vector3f::Dot(u, eye);
        Result[3][2] = Vector3f::Dot(f, eye);
        return Result;
    }

}
namespace apatite
{
    using math::Vector2f;
    using math::Vector2i;
    using math::Vector3f;
    using math::Vector3i;
    using math::Vector4f;
    using math::Vector4i;
    using math::Quat4f;
    using math::Matrix2f;
    using math::Matrix3f;
    using math::Matrix4f;
    using math::Color8b4;
    using math::LinearColorf;
    using math::Transform3Df;
}
