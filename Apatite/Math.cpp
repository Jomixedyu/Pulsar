#include "Math.h"

namespace apatite::math
{
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, Vector2f& vec)
    {
        ser::ReadWriteStream(stream, is_write, vec.x);
        ser::ReadWriteStream(stream, is_write, vec.y);
        return stream;
    }
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, Vector3f& vec)
    {
        ser::ReadWriteStream(stream, is_write, vec.x);
        ser::ReadWriteStream(stream, is_write, vec.y);
        ser::ReadWriteStream(stream, is_write, vec.z);
        return stream;
    }
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, Vector4f& vec)
    {
        ser::ReadWriteStream(stream, is_write, vec.x);
        ser::ReadWriteStream(stream, is_write, vec.y);
        ser::ReadWriteStream(stream, is_write, vec.z);
        ser::ReadWriteStream(stream, is_write, vec.w);
        return stream;
    }
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, Quat4f& q)
    {
        ser::ReadWriteStream(stream, is_write, q.w);
        ser::ReadWriteStream(stream, is_write, q.x);
        ser::ReadWriteStream(stream, is_write, q.y);
        ser::ReadWriteStream(stream, is_write, q.z);
        return stream;
    }
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, Color8b4& c)
    {
        ser::ReadWriteStream(stream, is_write, c.r);
        ser::ReadWriteStream(stream, is_write, c.g);
        ser::ReadWriteStream(stream, is_write, c.b);
        ser::ReadWriteStream(stream, is_write, c.a);
        return stream;
    }
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, LinearColorf& c)
    {
        ser::ReadWriteStream(stream, is_write, c.r);
        ser::ReadWriteStream(stream, is_write, c.g);
        ser::ReadWriteStream(stream, is_write, c.b);
        ser::ReadWriteStream(stream, is_write, c.a);
        return stream;
    }
}