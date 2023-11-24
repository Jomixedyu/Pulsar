#pragma once
#include <Pulsar/ObjectBase.h>

namespace pulsar
{

    class IShadeParameters
    {
    public:
        virtual int32_t GetUniformLocaltion(string_view name) = 0;
        virtual void SetUniformInt(string_view name, const int32_t& i) = 0;
        virtual void SetUniformFloat(string_view name, const float& f) = 0;
        virtual void SetUniformMatrix4fv(string_view name, const float* value) = 0;
        virtual void SetUniformMatrix4fv(string_view name, const Matrix4f& mat) = 0;
        virtual void SetUniformVector3(string_view name, const Vector3f& value) = 0;
        virtual void SetUniformColor(string_view name, const Color4f& value) = 0;
        virtual void SetUniformColor(string_view name, const Vector3f& value) = 0;
    };
}