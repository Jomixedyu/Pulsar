#include "EngineMath.h"

namespace jmath
{
    using namespace jxcorlib;

    std::iostream& ReadWriteStream(std::iostream& stream, bool is_write, Vector2f& vec)
    {
        sser::ReadWriteStream(stream, is_write, vec.x);
        sser::ReadWriteStream(stream, is_write, vec.y);
        return stream;
    }
    std::iostream& ReadWriteStream(std::iostream& stream, bool is_write, Vector3f& vec)
    {
        sser::ReadWriteStream(stream, is_write, vec.x);
        sser::ReadWriteStream(stream, is_write, vec.y);
        sser::ReadWriteStream(stream, is_write, vec.z);
        return stream;
    }
    std::iostream& ReadWriteStream(std::iostream& stream, bool is_write, Vector4f& vec)
    {
        sser::ReadWriteStream(stream, is_write, vec.x);
        sser::ReadWriteStream(stream, is_write, vec.y);
        sser::ReadWriteStream(stream, is_write, vec.z);
        sser::ReadWriteStream(stream, is_write, vec.w);
        return stream;
    }
    std::iostream& ReadWriteStream(std::iostream& stream, bool is_write, Quat4f& q)
    {
        sser::ReadWriteStream(stream, is_write, q.w);
        sser::ReadWriteStream(stream, is_write, q.x);
        sser::ReadWriteStream(stream, is_write, q.y);
        sser::ReadWriteStream(stream, is_write, q.z);
        return stream;
    }
    std::iostream& ReadWriteStream(std::iostream& stream, bool is_write, Color4b& c)
    {
        sser::ReadWriteStream(stream, is_write, c.r);
        sser::ReadWriteStream(stream, is_write, c.g);
        sser::ReadWriteStream(stream, is_write, c.b);
        sser::ReadWriteStream(stream, is_write, c.a);
        return stream;
    }
    std::iostream& ReadWriteStream(std::iostream& stream, bool is_write, Color4f& c)
    {
        sser::ReadWriteStream(stream, is_write, c.r);
        sser::ReadWriteStream(stream, is_write, c.g);
        sser::ReadWriteStream(stream, is_write, c.b);
        sser::ReadWriteStream(stream, is_write, c.a);
        return stream;
    }
}