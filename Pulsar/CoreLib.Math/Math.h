#pragma once

#include "Jmath.h"
#include <CoreLib/Type.h>
#include <CoreLib/Reflection.h>
#include <format>
#include "Assembly.h"


namespace jxcorlib::math
{
    using namespace jmath;

    //class BoxingColor8b4 : public jxcorlib::BoxingObject
    //{
    //    CORELIB_DEF_TYPE(AssemblyObject_JxMath, jxcorlib::math::BoxingColor8b4, jxcorlib::BoxingObject);
    //public:

    //    CORELIB_REFL_DECL_FIELD(r);
    //    int r;
    //    CORELIB_REFL_DECL_FIELD(g);
    //    int g;
    //    CORELIB_REFL_DECL_FIELD(b);
    //    int b;
    //    CORELIB_REFL_DECL_FIELD(a);
    //    int a;

    //    using unboxing_type = Color8b4;
    //    Vector2f get_unboxing_value() { return Color8b4(r, g, b, a); }

    //    BoxingColor8b4() : r(0), g(0), b(0), a(0) {}
    //    BoxingColor8b4(Color8b4 value) : r(value.r), g(value.g), b(value.b), a(value.a) {}

    //    virtual string ToString() const override { return to_string(unboxing_type(r, g, b, a)); }
    //};
    //template<> struct get_boxing_type<Color8b4> { using type = BoxingColor8b4; };


    class BoxingLinearColorf : public jxcorlib::BoxingObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_JxMath, jxcorlib::math::BoxingLinearColorf, jxcorlib::BoxingObject);
    public:

        CORELIB_REFL_DECL_FIELD(r);
        float r;
        CORELIB_REFL_DECL_FIELD(g);
        float g;
        CORELIB_REFL_DECL_FIELD(b);
        float b;
        CORELIB_REFL_DECL_FIELD(a);
        float a;

        using unboxing_type = LinearColorf;
        LinearColorf get_unboxing_value() { return LinearColorf(r, g, b, a); }

        BoxingLinearColorf() : r(0), g(0), b(0), a(0) {}
        BoxingLinearColorf(LinearColorf value) : r(value.r), g(value.g), b(value.b), a(value.a) {}

        virtual string ToString() const override { return to_string(unboxing_type(r, g, b, a)); }
    };
    template<> struct get_boxing_type<LinearColorf> { using type = BoxingLinearColorf; };


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


    class BoxingMatrix4f : public jxcorlib::BoxingObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_JxMath, jxcorlib::math::BoxingMatrix4f, jxcorlib::BoxingObject);
    public:
        CORELIB_REFL_DECL_FIELD(v1x);
        float v1x;
        CORELIB_REFL_DECL_FIELD(v1y);
        float v1y;
        CORELIB_REFL_DECL_FIELD(v1z);
        float v1z;
        CORELIB_REFL_DECL_FIELD(v1w);
        float v1w;

        CORELIB_REFL_DECL_FIELD(v2x);
        float v2x;
        CORELIB_REFL_DECL_FIELD(v2y);
        float v2y;
        CORELIB_REFL_DECL_FIELD(v2z);
        float v2z;
        CORELIB_REFL_DECL_FIELD(v2w);
        float v2w;

        CORELIB_REFL_DECL_FIELD(v3x);
        float v3x;
        CORELIB_REFL_DECL_FIELD(v3y);
        float v3y;
        CORELIB_REFL_DECL_FIELD(v3z);
        float v3z;
        CORELIB_REFL_DECL_FIELD(v3w);
        float v3w;

        CORELIB_REFL_DECL_FIELD(v4x);
        float v4x;
        CORELIB_REFL_DECL_FIELD(v4y);
        float v4y;
        CORELIB_REFL_DECL_FIELD(v4z);
        float v4z;
        CORELIB_REFL_DECL_FIELD(v4w);
        float v4w;

        using unboxing_type = Matrix4f;
        Matrix4f get_unboxing_value() const
        { 
            return Matrix4f({ v1x,v1y,v1z,v1w }, { v2x,v2y,v2z,v2w }, { v3x,v3y,v3z,v3w }, { v4x,v4y,v4z,v4w });
        }

        BoxingMatrix4f() {}
        BoxingMatrix4f(const Matrix4f& value)
        {
            v1x = value[0][0]; v2x = value[1][0]; v3x = value[2][0]; v4x = value[3][0];
            v1y = value[0][1]; v2y = value[1][1]; v3y = value[2][1]; v4y = value[3][1];
            v1z = value[0][2]; v2z = value[1][2]; v3z = value[2][2]; v4z = value[3][2];
            v1w = value[0][3]; v2w = value[1][3]; v3w = value[2][3]; v4w = value[3][3];
        }

        virtual string ToString() const override { return to_string(this->get_unboxing_value()); }
    };
    template<> struct get_boxing_type<Matrix4f> { using type = BoxingMatrix4f; };
}