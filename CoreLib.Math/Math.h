#pragma once

#include "Jmath.h"
#include <CoreLib/Type.h>
#include <CoreLib/Reflection.h>
#include <format>
#include "Assembly.h"


namespace jxcorlib::math
{
    using namespace jmath;

    class BoxingVector2f : public jxcorlib::BoxingObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_JxMath, jxcorlib::math::BoxingVector2f, jxcorlib::BoxingObject);
    public:

        CORELIB_REFL_DECL_FIELD(x);
        float x;
        CORELIB_REFL_DECL_FIELD(y);
        float y;

        using unboxing_type = Vector2f;
        Vector2f get_unboxing_value() { return Vector2f(x, y); }

        BoxingVector2f() : x(0), y(0) {}
        BoxingVector2f(Vector2f value) : x(value.x), y(value.y) {}

        virtual string ToString() const override { return to_string(unboxing_type(x, y)); }
    };
    template<> struct get_boxing_type<Vector2f> { using type = BoxingVector2f; };


    class BoxingVector3f : public jxcorlib::BoxingObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_JxMath, jxcorlib::math::BoxingVector3f, jxcorlib::BoxingObject);
    public:

        CORELIB_REFL_DECL_FIELD(x);
        float x;
        CORELIB_REFL_DECL_FIELD(y);
        float y;
        CORELIB_REFL_DECL_FIELD(z);
        float z;

        using unboxing_type = Vector3f;
        Vector3f get_unboxing_value() { return Vector3f(x, y, z); }

        BoxingVector3f() : x(0), y(0), z(0) {}
        BoxingVector3f(Vector3f value) : x(value.x), y(value.y), z(value.z) {}

        virtual string ToString() const override { return to_string(unboxing_type(x, y, z)); }
    };
    template<> struct get_boxing_type<Vector3f> { using type = BoxingVector3f; };

    class BoxingVector4f : public jxcorlib::BoxingObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_JxMath, jxcorlib::math::BoxingVector4f, jxcorlib::BoxingObject);
    public:

        CORELIB_REFL_DECL_FIELD(x);
        float x;
        CORELIB_REFL_DECL_FIELD(y);
        float y;
        CORELIB_REFL_DECL_FIELD(z);
        float z;
        CORELIB_REFL_DECL_FIELD(w);
        float w;

        using unboxing_type = Vector4f;
        Vector4f get_unboxing_value() { return Vector4f(x, y, z, w); }

        BoxingVector4f() : x(0), y(0), z(0), w(0) {}
        BoxingVector4f(Vector4f value) : x(value.x), y(value.y), z(value.z), w(value.w) {}

        virtual string ToString() const override { return to_string(unboxing_type(x, y, z, w)); }
    };
    template<> struct get_boxing_type<Vector4f> { using type = BoxingVector4f; };

    class BoxingQuat4f : public jxcorlib::BoxingObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_JxMath, jxcorlib::math::BoxingQuat4f, jxcorlib::BoxingObject);
    public:

        CORELIB_REFL_DECL_FIELD(w);
        float w;
        CORELIB_REFL_DECL_FIELD(x);
        float x;
        CORELIB_REFL_DECL_FIELD(y);
        float y;
        CORELIB_REFL_DECL_FIELD(z);
        float z;

        using unboxing_type = Quat4f;
        Quat4f get_unboxing_value() { return Quat4f(x, y, z, w); }

        BoxingQuat4f() : w(0), x(0), y(0), z(0) {}
        BoxingQuat4f(Quat4f value) : w(value.w), x(value.x), y(value.y), z(value.z) {}

        virtual string ToString() const override { return to_string(unboxing_type(w, x, y, z)); }
    };
    template<> struct get_boxing_type<Quat4f> { using type = BoxingQuat4f; };

}