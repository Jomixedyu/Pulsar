#pragma once
#include <CoreLib.Math/Math.h>
#include <CoreLib/sser.hpp>

namespace jmath
{
    std::iostream& ReadWriteStream(std::iostream& stream, bool is_write, Vector2f& vec);
    std::iostream& ReadWriteStream(std::iostream& stream, bool is_write, Vector3f& vec);
    std::iostream& ReadWriteStream(std::iostream& stream, bool is_write, Vector4f& vec);
    std::iostream& ReadWriteStream(std::iostream& stream, bool is_write, Quat4f& q);
    std::iostream& ReadWriteStream(std::iostream& stream, bool is_write, Color4b& c);
    std::iostream& ReadWriteStream(std::iostream& stream, bool is_write, Color4f& c);
}

namespace pulsar::math
{
    using namespace jxcorlib;
    using namespace jxcorlib::math;

    inline float LinearToSRGB(float x)
    {
        return x < 0.0031308f ? 12.92f * x : 1.055f * powf(x, 1.0f / 2.4f) - 0.055f;
    }
    inline float SRGBToLinear(float x)
    {
        return x < 0.04045f ? x / 12.92f : powf((x + 0.055f) / 1.055f, 2.4f);
    }
    inline Vector3f LinearToSRGB(Vector3f c)
    {
        return { LinearToSRGB(c.x), LinearToSRGB(c.y), LinearToSRGB(c.z) };
    }
    inline Vector3f SRGBToLinear(Vector3f c)
    {
        return { SRGBToLinear(c.x), SRGBToLinear(c.y), SRGBToLinear(c.z) };
    }

    inline void Perspective(Matrix4f& result, const float& fovy, const float& aspect, const float& zNear, const float& zFar)
    {
        float const tanHalfFovy = tan(fovy / 2);
        result = Matrix4f{0};
        result[0][0] = 1 / (aspect * tanHalfFovy);
        result[1][1] = 1 / (tanHalfFovy);
        result[2][3] = -1;

        result[2][2] = -(zFar + zNear) / (zFar - zNear);
        result[3][2] = -(2 * zFar * zNear) / (zFar - zNear);
    }
    inline void Perspective_LHZO(Matrix4f& result, const float& fovy, const float& aspect, const float& zNear, const float& zFar)
    {
        float const tanHalfFovy = tan(fovy / 2);
        result = Matrix4f{0};
        result[0][0] = 1 / (aspect * tanHalfFovy);
        result[1][1] = 1 / (tanHalfFovy);
        result[2][2] = zFar / (zFar - zNear);
        result[2][3] = 1;
        result[3][2] = -(zFar * zNear) / (zFar - zNear);
    }

    inline void Ortho(Matrix4f& Result, float left, float right, float bottom, float top, float zNear, float zFar)
    {
        Result[0][0] = 2 / (right - left);
        Result[1][1] = 2 / (top - bottom);
        Result[3][0] = -(right + left) / (right - left);
        Result[3][1] = -(top + bottom) / (top - bottom);

        Result[2][2] = -2 / (zFar - zNear);
        Result[3][2] = -(zFar + zNear) / (zFar - zNear);
    }
    inline void Ortho_LHZO(Matrix4f& Result, float left, float right, float bottom, float top, float zNear, float zFar)
    {
        Result[0][0] = static_cast<float>(2) / (right - left);
        Result[1][1] = static_cast<float>(2) / (top - bottom);
        Result[2][2] = static_cast<float>(1) / (zFar - zNear);
        Result[3][0] = - (right + left) / (right - left);
        Result[3][1] = - (top + bottom) / (top - bottom);
        Result[3][2] = - zNear / (zFar - zNear);
    }


}
namespace pulsar
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
    using math::Color4b;
    using math::Color4f;
    using math::Transform3Df;
    using math::BoxBounds3f;
    using math::BoxSphereBounds3f;
    using math::SphereBounds3f;
    using math::Triangle3f;
}
