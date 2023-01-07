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

}
namespace apatite
{
    using math::Vector2f;
    using math::Vector3f;
    using math::Vector4f;
    using math::Quat4f;
    using math::Matrix2f;
    using math::Matrix3f;
    using math::Matrix4f;
    using math::Color8b4;
    using math::LinearColorf;
    using math::Transform3Df;
}
