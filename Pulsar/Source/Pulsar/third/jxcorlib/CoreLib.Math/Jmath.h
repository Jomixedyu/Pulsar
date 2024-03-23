#pragma once
#include <cmath>
#include <string>
#include <type_traits>
#include <cassert>
#include <cstdint>
#include <algorithm>

namespace jmath
{
    template<typename T = float> inline constexpr T pi() { return static_cast<T>(3.14159265358979323846264338327950288); }

    template<typename T = float> inline constexpr T deg2rad() { return static_cast<T>(0.017453292519943f); }
    template<typename T = float> inline constexpr T rad2deg() { return static_cast<T>(57.295779513082320876798154814105f); }

    template<typename T> constexpr T Radians(T degree)
    {
        if constexpr (std::is_arithmetic_v<T>)
            return deg2rad<T>() * degree;
        else
            return deg2rad<typename T::value_type>() * degree;
    }

    template<typename T> constexpr T Degrees(T rad) { return rad2deg<typename T::value_type>() * rad; }
    template<typename T> constexpr T Clamp(T x, T min, T max)
    {
        return std::min(std::max(x, min), max);
    }
    template<typename T>
    constexpr bool FloatEqual(T const& x, T const& y)
    {
        return std::abs(x - y) <= std::numeric_limits<T>::epsilon();
    }

    inline float Chgsign(float x, float y)
    {
        auto a = *reinterpret_cast<int*>(&x) ^ *reinterpret_cast<int*>(&y) & (int)0x80000000;
        return *reinterpret_cast<float*>(&a);
    }


    template<typename T>
    struct Vector2
    {
        using value_type = T;

        T x, y;

        constexpr Vector2() = default;
        constexpr Vector2(T _x, T _y) : x(_x), y(_y) {}

        template<typename U>
        constexpr Vector2(const U& r) : x((T)r.x), y((T)r.y) {}

        const T* get_value_ptr() const { return &this->x; }
        T* get_value_ptr() { return &this->x; }

        static constexpr int column_count = 1;
        static constexpr int row_count = 2;

        T& operator[](int index) { return *(&x + index); }
        const T& operator[](int index) const { return *(&x + index); }

        static Vector2 StaticZero() { return Vector2{ T(0), T(0) }; }
        static Vector2 StaticOne() { return Vector2{ T(1), T(1) }; }
        static Vector2 StaticUp() { return Vector2{ T(0), T(1) }; }
        static Vector2 StaticRight() { return Vector2{ T(1), T(0) }; }

        Vector2 operator-() { return Vector2(-x, -y); }
        Vector2 operator+=(Vector2 r) { x += r.x; y += r.y; return *this; }
        Vector2 operator+=(T r) { x += r; y += r; return *this; }
        Vector2 operator-=(Vector2 r) { x -= r.x; y -= r.y; return *this; }
        Vector2 operator-=(T r) { x -= r; y -= r; return *this; }
        Vector2 operator*=(T r) { x *= r; y *= r; return *this; }
        Vector2 operator/=(T r) { x /= r; y /= r; return *this; }

        static inline T Dot(const Vector2<T>& a, const Vector2<T>& b)
        {
            return a.x * b.x + a.y * b.y;
        }
        static Vector2<T> Normalize(const Vector2<T>& input);
        static void Normalized() { *this = Normalize(*this); }
        inline Vector2 Reflect(const Vector2& input, const Vector2& normal)
        {
            return -input + 2.0f * Dot(Normalize(input), normal) * normal;
        }
    };


    template<typename T> inline Vector2<T> operator*(Vector2<T> a, T b) { return Vector2<T>(a.x * b, a.y * b); }
    template<typename T> inline Vector2<T> operator*(T a, Vector2<T> b) { return Vector2<T>(a * b.x, a * b.y); }
    template<typename T> inline Vector2<T> operator*(Vector2<T> a, Vector2<T> b) { return Vector2<T>(a.x * b.x, a.y * b.y); }
    template<typename T> inline Vector2<T> operator/(Vector2<T> a, T b) { return Vector2<T>(a.x / b, a.y / b); }
    template<typename T> inline Vector2<T> operator/(T a, Vector2<T> b) { return Vector2<T>(a / b.x, a / b.y); }
    template<typename T> inline Vector2<T> operator+(Vector2<T> a, T b) { return Vector2<T>(a.x + b, a.y + b); }
    template<typename T> inline Vector2<T> operator+(T a, Vector2<T> b) { return Vector2<T>(a + b.x, a + b.y); }
    template<typename T> inline Vector2<T> operator+(Vector2<T> a, Vector2<T> b) { return Vector2<T>(a.x + b.x, a.y + b.y); }
    template<typename T> inline Vector2<T> operator-(Vector2<T> a, T b) { return Vector2<T>(a.x - b, a.y - b); }
    template<typename T> inline Vector2<T> operator-(T a, Vector2<T> b) { return Vector2<T>(a - b.x, a - b.y); }
    template<typename T> inline Vector2<T> operator-(Vector2<T> a, Vector2<T> b) { return Vector2<T>(a.x - b.x, a.y - b.y); }
    template<typename T> inline bool operator==(Vector2<T> a, Vector2<T> b) { return a.x == b.x && a.y == b.y; }

    template<typename T>
    inline Vector2<T> Vector2<T>::Normalize(const Vector2<T>& input)
    {
        return input / sqrt(Dot(input, input));
    }

    template<typename T> std::string to_string(Vector2<T> v)
    {
        std::string s;
        s.reserve(64);
        s.append("{x: "); s.append(std::to_string(v.x)); s.append(", ");
        s.append("y: "); s.append(std::to_string(v.y)); s.append("}");
        return s;
    }

    using Vector2f = Vector2<float>;
    using Vector2d = Vector2<double>;
    using Vector2i = Vector2<int>;

    template<typename T>
    T SumComponents(const Vector2<T>& v) { return v.x + v.y; }


    template<typename T>
    struct Vector3
    {
        using value_type = T;

        T x, y, z;

        constexpr Vector3() = default;
        constexpr Vector3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}

        const T* get_value_ptr() const { return &this->x; }
        T* get_value_ptr() { return &this->x; }

        static constexpr int column_count = 1;
        static constexpr int row_count = 3;

        T& operator[](int index) { return *(&x + index); }
        const T& operator[](int index) const { return *(&x + index); }

        static Vector3 StaticUp() { return Vector3{ T(0), T(1), T(0) }; }
        static Vector3 StaticRight() { return Vector3{ T(1), T(0), T(0) }; }
        //right handle?
        static Vector3 StaticForward() { return Vector3{ T(0), T(0), T(1) }; }
        static Vector3 StaticZero() { return Vector3{ T(0), T(0), T(0) }; }
        static Vector3 StaticOne() { return Vector3{ T(1), T(1), T(1) }; }

        Vector3& operator +=(Vector3 v) { x += v.x; y += v.y; z += v.z; return *this; }
        Vector3& operator +=(T v) { x += v; y += v; z += v; return *this; }
        Vector3& operator -=(Vector3 v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
        Vector3& operator -=(T v) { x -= v; y -= v; z -= v; return *this; }
        Vector3& operator *=(T v) { x *= v; y *= v; z *= v; return *this; }
        Vector3& operator /=(T v) { x /= v; y /= v; z /= v; return *this; }
        Vector3 operator-() { return Vector3(-x, -y, -z); }

        static inline T Distance(const Vector3& l, const Vector3& r)
        {
            T x = l.x - r.x;
            T z = l.z - r.z;
            T y = l.y - r.y;
            return sqrt(x * x + y * y * z * z);
        }
        static inline T Dot(const Vector3<T>& l, const Vector3<T>& r)
        {
            return l.x * r.x + l.y * r.y + l.z * r.z;
        }
        static inline Vector3 Mul(const Vector3<T>& l, const Vector3<T>& r)
        {
            return { l.x * r.x, l.y * r.y, l.z * r.z };
        }
        static Vector3<T> Normalize(const Vector3<T>& target);
        void Normalized() { *this = Normalize(*this); }
        Vector3<T> GetNormalized() const { return Normalize(*this); }
        static inline Vector3<T> Cross(const Vector3<T>& target1, const Vector3<T>& target2)
        {
            return Vector3<T>(
                target1.y * target2.z - target1.z * target2.y,
                target1.z * target2.x - target1.x * target2.z,
                target1.x * target2.y - target1.y * target2.x
                );
        }
        T Magnitude() const
        {
            return sqrtf(Dot(*this, *this));
        }

        static Vector3 Identity() { return { T(1), T(1), T(1) }; }
    };
    template<typename T> inline Vector3<T> operator+(Vector3<T> l, T r) { return Vector3<T>(l.x + r, l.y + r, l.z + r); }
    template<typename T> inline Vector3<T> operator+(T l, Vector3<T> r) { return Vector3<T>(l + r.x, l + r.y, l + r.z); }
    template<typename T> inline Vector3<T> operator+(Vector3<T> l, Vector3<T> r) { return Vector3<T>(l.x + r.x, l.y + r.y, l.z + r.z); }
    template<typename T> inline Vector3<T> operator-(Vector3<T> l, T r) { return Vector3<T>(l.x - r, l.y - r, l.z - r); }
    template<typename T> inline Vector3<T> operator-(T l, Vector3<T> r) { return Vector3<T>(l - r.x, l - r.y, l - r.z); }
    template<typename T> inline Vector3<T> operator-(Vector3<T> l, Vector3<T> r) { return Vector3<T>(l.x - r.x, l.y - r.y, l.z - r.z); }
    template<typename T> inline Vector3<T> operator/(Vector3<T> v3, T f) { return Vector3<T>(v3.x / f, v3.y / f, v3.z / f); }
    template<typename T> inline Vector3<T> operator/(T f, Vector3<T> v3) { return Vector3<T>(f / v3.x, f / v3.y, f / v3.z); }
    template<typename T> inline Vector3<T> operator*(Vector3<T> v3, T f) { return Vector3<T>(v3.x * f, v3.y * f, v3.z * f); }
    template<typename T> inline Vector3<T> operator*(T f, Vector3<T> v3) { return Vector3<T>(v3.x * f, v3.y * f, v3.z * f); }
    template<typename T> inline Vector3<T> operator*(Vector3<T> l, Vector3<T> r) { return Vector3<T>(l.x * r.x, l.y * r.y, l.z * r.z); }
    template<typename T> inline bool operator==(Vector3<T> l, Vector3<T> r) { return l.x == r.x && l.y && r.y && l.z && r.z; }

    template<typename T>
    inline Vector3<T> Vector3<T>::Normalize(const Vector3<T>& target)
    {
        return target / sqrtf(Dot(target, target));
    }

    template<typename T> std::string to_string(const Vector3<T>& v)
    {
        std::string s;
        s.reserve(64);
        s.append("{x: "); s.append(std::to_string(v.x)); s.append(", ");
        s.append("y: "); s.append(std::to_string(v.y)); s.append(", ");
        s.append("z: "); s.append(std::to_string(v.z)); s.append("}");
        return s;
    }

    using Vector3f = Vector3<float>;
    constexpr int kSizeVector3f = sizeof(Vector3f);
    using Vector3d = Vector3<double>;
    constexpr int kSizeVector3d = sizeof(Vector3d);
    using Vector3i = Vector3<int>;
    constexpr int kSizeVector3i = sizeof(Vector3i);

    template<typename T>
    T SumComponents(const Vector3<T>& v) { return v.x + v.y + v.z; }

    template<typename T>
    Vector3<T> Chgsign(const Vector3<T>& a, const Vector3<T>& b)
    {
        return Vector3<T>{ Chgsign(a.x, b.x), Chgsign(a.y, b.y), Chgsign(a.z, b.z) };
    }



    template<typename T>
    struct Vector4
    {
        using value_type = T;

        T x, y, z, w;

        constexpr Vector4() = default;
        constexpr Vector4(const Vector3<T>& v3, T h) : x(v3.x), y(v3.y), z(v3.z), w(h) {}
        constexpr Vector4(const Vector3<T>& v3) : x(v3.x), y(v3.y), z(v3.z), w(1) {}
        constexpr Vector4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}

        const T* get_value_ptr() const { return &this->x; }
        T* get_value_ptr() { return &this->x; }

        static constexpr int column_count = 1;
        static constexpr int row_count = 4;

        static constexpr Vector4 Mul(const Vector4<T>& l, const Vector4<T>& r)
        {
            return { l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w };
        }

        constexpr Vector3<T> xyz() const { return {x,y,z}; }

        T& operator[](int index) { return *(&x + index); }
        const T& operator[](int index) const { return *(&x + index); }
    };

    template<typename T> std::string to_string(const Vector4<T>& v)
    {
        std::string s;
        s.reserve(64);
        s.append("{x: "); s.append(std::to_string(v.x)); s.append(", ");
        s.append("y: "); s.append(std::to_string(v.y)); s.append(", ");
        s.append("z: "); s.append(std::to_string(v.z)); s.append(", ");
        s.append("w: "); s.append(std::to_string(v.w)); s.append("}");
        return s;
    }

    template<typename T> Vector4<T> operator+(const Vector4<T>& a, const Vector4<T>& b) { return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
    template<typename T> Vector4<T> operator-(const Vector4<T>& a, const Vector4<T>& b) { return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
    template<typename T> Vector4<T> operator*(const Vector4<T>& a, const Vector4<T>& b) { return { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w }; }
    template<typename T> Vector4<T> operator/(const Vector4<T>& a, const Vector4<T>& b) { return { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w }; }
    template<typename T> Vector4<T> operator+(const Vector4<T>& a, T b) { return { a.x + b, a.y + b, a.z + b, a.w + b }; }
    template<typename T> Vector4<T> operator-(const Vector4<T>& a, T b) { return { a.x - b, a.y - b, a.z - b, a.w - b }; }
    template<typename T> Vector4<T> operator*(const Vector4<T>& a, T b) { return { a.x * b, a.y * b, a.z * b, a.w * b }; }
    template<typename T> Vector4<T> operator/(const Vector4<T>& a, T b) { return { a.x / b, a.y / b, a.z / b, a.w / b }; }
    template<typename T> Vector4<T> operator+(T a, const Vector4<T>& b) { return { a + b.x, a + b.y, a + b.z, a + b.w }; }
    template<typename T> Vector4<T> operator-(T a, const Vector4<T>& b) { return { a - b.x, a - b.y, a - b.z, a - b.w }; }
    template<typename T> Vector4<T> operator*(T a, const Vector4<T>& b) { return { a * b.x, a * b.y, a * b.z, a * b.w }; }
    template<typename T> Vector4<T> operator/(T a, const Vector4<T>& b) { return { a / b.x, a / b.y, a / b.z, a / b.w }; }
    template<typename T> bool operator==(const Vector4<T>& a, const Vector4<T>& b) { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }

    using Vector4f = Vector4<float>;
    using Vector4d = Vector4<double>;
    using Vector4i = Vector4<int>;

    template<typename T>
    T SumComponents(const Vector4<T>& v) { return v.x + v.y + v.z + v.w; }



    template<typename T>
    struct Matrix2
    {
        using value_type = T;
        Vector2<T> M[2];

        Matrix2(
            T ax, T bx,
            T ay, T by) {
            M[0] = Vector2<T>(ax, ay);
            M[1] = Vector2<T>(bx, by);
        }
        Matrix2(Vector2<T> x, Vector2<T> y) { M[0] = x; M[1] = y; }

        static constexpr int column_count = 2;
        static constexpr int row_count = 2;

        const T* get_value_ptr() const { return &M[0].x; }
        T* get_value_ptr() { return &M[0].x; }

        Vector2<T>& operator[](int i) { return M[i]; }
        const Vector2<T>& operator[](int i) const { return M[i]; }

        static Matrix2 StaticScalar(T k = T(1))
        {
            return Matrix2(
                T(k), T(0),
                T(0), T(k)
            );
        }
    };
    using Matrix2f = Matrix2<float>;
    using Matrix2d = Matrix2<double>;
    using Matrix2i = Matrix2<int>;

    template<typename T>
    struct Matrix3
    {
        using value_type = T;

        Vector3<T> M[3];

        const T* get_value_ptr() const { return &M[0].x; }
        T* get_value_ptr() { return &M[0].x; }

        Matrix3(
            T ax, T bx, T cx,
            T ay, T by, T cy,
            T az, T bz, T cz
        ) {
            M[0] = Vector3<T>(ax, ay, az);
            M[1] = Vector3<T>(bx, by, bz);
            M[2] = Vector3<T>(cx, cy, cz);
        }
        Matrix3(Vector3<T> x, Vector3<T> y, Vector3<T> z) { M[0] = x; M[1] = y; M[2] = z; }

        static constexpr int column_count = 3;
        static constexpr int row_count = 3;

        Vector3<T>& operator[](int i) { return M[i]; }
        const Vector3<T>& operator[](int i) const { return M[i]; }

        static Matrix3 StaticScalar(T k = T(1))
        {
            return Matrix3(
                T(k), T(0), T(0),
                T(0), T(k), T(0),
                T(0), T(0), T(k)
            );
        }
    };
    using Matrix3f = Matrix3<float>;
    using Matrix3d = Matrix3<double>;
    using Matrix3i = Matrix3<int>;

    template<typename T>
    struct Matrix4
    {
        using value_type = T;

        Vector4<T> M[4];

        const T* get_value_ptr() const { return &M[0].x; }
        T* get_value_ptr() { return &M[0].x; }

        Matrix4() = default;

        constexpr explicit Matrix4(T k)
        {
            M[0] = {T(k),T(0),T(0),T(0)};
            M[1] = {T(0),T(k),T(0),T(0)};
            M[2] = {T(0),T(0),T(k),T(0)};
            M[3] = {T(0),T(0),T(0),T(k)};
        }

        constexpr Matrix4(
            T ax, T bx, T cx, T dx,
            T ay, T by, T cy, T dy,
            T az, T bz, T cz, T dz,
            T aw, T bw, T cw, T dw
        ) {
            M[0] = Vector4<T>(ax, ay, az, aw);
            M[1] = Vector4<T>(bx, by, bz, bw);
            M[2] = Vector4<T>(cx, cy, cz, cw);
            M[3] = Vector4<T>(dx, dy, dz, dw);
        }
        constexpr Matrix4(const Vector4<T>& x, const Vector4<T>& y, const Vector4<T>& z, const Vector4<T>& w) { M[0] = x; M[1] = y; M[2] = z; M[3] = w; }

        static constexpr int column_count = 4;
        static constexpr int row_count = 4;

        Vector4<T>& operator[](int i) { return M[i]; }
        const Vector4<T>& operator[](int i) const { return M[i]; }

        Vector4<T> GetColumn(int i) const { return M[i]; }
        Vector4<T> GetRow(int i) const { return Vector4<T>{ M[0][i], M[1][i], M[2][i], M[3][i] }; }

        T Determinant() const
        {
            return	M[0][0] * (
                M[1][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
                M[2][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) +
                M[3][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2])
                ) -
                M[1][0] * (
                    M[0][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
                    M[2][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
                    M[3][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2])
                    ) +
                M[2][0] * (
                    M[0][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) -
                    M[1][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
                    M[3][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
                    ) -
                M[3][0] * (
                    M[0][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2]) -
                    M[1][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2]) +
                    M[2][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
                    );
        }
    };

    template<typename T>
    Matrix4<T> Inverse(const Matrix4<T>& m)
    {
        T Coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		T Coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
		T Coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

		T Coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		T Coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
		T Coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

		T Coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		T Coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
		T Coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

		T Coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		T Coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
		T Coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

		T Coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		T Coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
		T Coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

		T Coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
		T Coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
		T Coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

		Vector4<T> Fac0(Coef00, Coef00, Coef02, Coef03);
		Vector4<T> Fac1(Coef04, Coef04, Coef06, Coef07);
		Vector4<T> Fac2(Coef08, Coef08, Coef10, Coef11);
		Vector4<T> Fac3(Coef12, Coef12, Coef14, Coef15);
		Vector4<T> Fac4(Coef16, Coef16, Coef18, Coef19);
		Vector4<T> Fac5(Coef20, Coef20, Coef22, Coef23);
		Vector4<T> Vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
		Vector4<T> Vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
		Vector4<T> Vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
		Vector4<T> Vec3(m[1][3], m[0][3], m[0][3], m[0][3]);
		Vector4<T> Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
		Vector4<T> Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
		Vector4<T> Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
		Vector4<T> Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

		Vector4<T> SignA(+1, -1, +1, -1);
		Vector4<T> SignB(-1, +1, -1, +1);
		Matrix4<T> Inverse(Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB);

		Vector4<T> Row0(Inverse[0][0], Inverse[1][0], Inverse[2][0], Inverse[3][0]);

		Vector4<T> Dot0(m[0] * Row0);
		T Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);

		T OneOverDeterminant = static_cast<T>(1) / Dot1;

		return Inverse * OneOverDeterminant;
    }
    template<typename T>
    Matrix4<T> Transpose(const Matrix4<T>& mat)
    {
        return Matrix4<T> { mat.GetRow(0), mat.GetRow(1), mat.GetRow(2), mat.GetRow(3) };
    }

    template<typename MAT>
    inline void Transpose(MAT* mat)
    {
        static_assert(MAT::column_count == MAT::row_count);

        for (int c = 0; c < MAT::column_count; c++)
        {
            for (int r = 0; r < MAT::column_count; r++)
            {
                if (c == r) break;
                auto a = mat->M[c][r];
                mat->M[c][r] = mat->M[r][c];
                mat->M[r][c] = a;
            }
        }
    }

    template<typename T>
    inline std::string to_string(const Matrix4<T>& v)
    {
        std::string s;
        s.reserve(64);
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                s.append(std::to_string(v[c][r]));
                if (c != 3)
                    s.append(", ");
            }
            s.append("\n");
        }
        return s;
    }

    template<typename T>
    Matrix4<T> operator*(const Matrix4<T>& a, const Matrix4<T>& b)
    {
        Matrix4<T> m;
        for (int l = 0; l < 4; l++)
        {
            for (int r = 0; r < 4; r++)
            {
                m[l][r] = SumComponents(a.GetRow(r) * b.GetColumn(l));
            }
        }
        return m;
    }

    template<typename T>
    Vector4<T> operator*(const Matrix4<T>& a, const Vector4<T>& b)
    {
        Vector4<T> v;
        for (int r = 0; r < 4; r++)
        {
            v[r] = SumComponents(Vector4<T>::Mul(a.GetRow(r), b));
        }
        return v;
    }
    template<typename T>
    Vector3<T> operator*(const Matrix4<T>& a, const Vector3<T>& b)
    {
        Vector4<T> v;
        for (int r = 0; r < 4; r++)
        {
            v[r] = SumComponents(Vector4<T>::Mul(a.GetRow(r), Vector4<T>(b)));
        }
        return v.xyz();
    }

    template<typename T>
    Matrix4<T> operator*(const Matrix4<T>& m, T scalar)
    {
        Matrix4<T> mat;
        for (int i = 0; i < 4; ++i)
        {
            mat.M[i] = m.M[i] * scalar;
        }
        return mat;
    }

    using Matrix4f = Matrix4<float>;
    using Matrix4d = Matrix4<double>;
    using Matrix4i = Matrix4<int>;

    template<typename Q, typename T>
    void EulerYXZToQuat(Q& quat, T pitch, T yaw, T roll)
    {
        const T halfpitch = pitch * 0.5f;
        T cp = cosf(halfpitch);
        T sp = sinf(halfpitch);

        const T halfyaw = yaw * 0.5f;
        T cy = cosf(halfyaw);
        T sy = sinf(halfyaw);

        const T halfroll = roll * 0.5f;
        T cr = cosf(halfroll);
        T sr = sinf(halfroll);

        quat.w = cr * cp * cy + sr * sp * sy;
        quat.x = cr * sp * cy + sr * cp * sy;
        quat.y = cr * cp * sy - sr * sp * cy;
        quat.z = sr * cp * cy - cr * sp * sy;
    }
    template <typename T>
    void ExtractEulerAngleYXZ(const Matrix4<T>& M,
                                                 T & t1,
                                                 T & t2,
                                                 T & t3)
    {
        T T1 = std::atan2(M[2][0], M[2][2]);
        T C2 = std::sqrt(M[0][1]*M[0][1] + M[1][1]*M[1][1]);
        T T2 = std::atan2(-M[2][1], C2);
        T S1 = std::sin(T1);
        T C1 = std::cos(T1);
        T T3 = std::atan2(S1*M[1][2] - C1*M[1][0], C1*M[0][0] - S1*M[0][2]);
        t1 = T1;
        t2 = T2;
        t3 = T3;
    }

    enum class EulerOrder : uint8_t
    {
        YXZ
    };
    template<typename T>
    struct Quaternion
    {
        using value_type = T;

        T x, y, z, w;

        Quaternion() : x(0), y(0), z(0), w(1) {}
        Quaternion(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}
        Quaternion(const Quaternion&) = default;
        Quaternion& operator=(const Quaternion& q) = default;

    public:
        static Quaternion Identity()
        {
            return Quaternion(T(0), T(0), T(0), T(1));
        }
        Quaternion& operator*=(const Quaternion& q)
        {
            Quaternion& p = *this;

            T _x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
            T _y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
            T _z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;
            T _w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;

            p.x = _x; p.y = _y; p.z = _z; p.w = _w;

            return *this;
        }
        Quaternion& operator/=(const T& scalar)
        {
            x /= scalar; y /= scalar; z /= scalar; w /= scalar;
            return *this;
        }
        friend Quaternion operator/(const Quaternion& in_q, const T& scalar)
        {
            Quaternion q = in_q;
            q /= scalar;
            return q;
        }
        Quaternion operator*(const Quaternion& q) const
        {
            Quaternion nq = *this;
            nq *= q;
            return nq;
        }

        static T Dot(const Quaternion& q1, const Quaternion& q2)
        {
            return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
        }
        static T Magnitude(const Quaternion& q)
        {
            return std::sqrt(Dot(q, q));
        }
        static Quaternion FromAxisQuaternions(const Quaternion& q1, const Quaternion& q2, const Quaternion& q3)
        {
            return (q1 * q2) * q3;
        }
        static Quaternion NormalizeSafe(const Quaternion& q)
        {
            T mag = Magnitude(q);
            if (mag < 1e-6f)
                return Quaternion::Identity();
            else
                return q / mag;
        }
        Vector3<T> GetEulerRad(EulerOrder order = EulerOrder::YXZ) const
        {
            Vector3<T> angles;
            ExtractEulerAngleYXZ(ToMatrix(), angles.y, angles.x, angles.z);
            return angles;
        }
        Vector3<T> GetEuler(EulerOrder order = EulerOrder::YXZ) const
        {
            return Degrees(GetEulerRad(order));
        }
        static Quaternion FromEuler(const Vector3<T>& euler, EulerOrder order = EulerOrder::YXZ)
        {
            Quaternion q;
            q.SetEulerRad(euler * (jmath::pi<T>() / T(180)), order);
            return q;
        }

        Matrix4<T> ToMatrix() const
        {
            Matrix4 Result(T(1));
            T qxx(x * x);
            T qyy(y * y);
            T qzz(z * z);
            T qxz(x * z);
            T qxy(x * y);
            T qyz(y * z);
            T qwx(w * x);
            T qwy(w * y);
            T qwz(w * z);

            Result[0][0] = T(1) - T(2) * (qyy +  qzz);
            Result[0][1] = T(2) * (qxy + qwz);
            Result[0][2] = T(2) * (qxz - qwy);

            Result[1][0] = T(2) * (qxy - qwz);
            Result[1][1] = T(1) - T(2) * (qxx +  qzz);
            Result[1][2] = T(2) * (qyz + qwx);

            Result[2][0] = T(2) * (qxz + qwy);
            Result[2][1] = T(2) * (qyz - qwx);
            Result[2][2] = T(1) - T(2) * (qxx +  qyy);

            Result[3][0] = T(0);
            Result[3][1] = T(0);
            Result[3][2] = T(0);
            Result[3][3] = T(1);

            return Result;
        }
    private:
        void SetEulerRad(const Vector3<T>& euler, EulerOrder order)
        {
            if(order == EulerOrder::YXZ)
            {
                EulerYXZToQuat(*this, euler.x, euler.y, euler.z);
            }
            else
                assert(false);
        }
        inline static Vector3<T> Internal_MakePositive(Vector3<T> euler)
        {
            float num = -0.005729578f;
            float num2 = 360.f + num;
            if (euler.x < num) { euler.x += 360.f; }
            else if (euler.x > num2) { euler.x -= 360.f; }
            if (euler.y < num) { euler.y += 360.f; }
            else if (euler.y > num2) { euler.y -= 360.f; }
            if (euler.z < num) { euler.z += 360.f; }
            else if (euler.z > num2) { euler.z -= 360.f; }
            return euler;
        }

    public:
        friend Quaternion operator-(const Quaternion& a, const Quaternion& b)
        {
            return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
        }
    };

    template<typename T>
    Quaternion<T> Chgsign(const Quaternion<T>& a, const Quaternion<T>& b)
    {
        return Quaternion<T>{ Chgsign(a.x, b.x), Chgsign(a.y, b.y), Chgsign(a.z, b.z), Chgsign(a.w, b.w) };
    }

    template<typename T>
    Matrix4<T> Translate(const Vector3<T>& v)
    {
        Matrix4<T> m{1};
        m[3][0] = v.x;
        m[3][1] = v.y;
        m[3][2] = v.z;
        return m;
    }

    template<typename T>
    Matrix4<T> Rotate(const Quaternion<T>& q)
    {
        Matrix4<T> Result{1};
        T qxx(q.x * q.x);
        T qyy(q.y * q.y);
        T qzz(q.z * q.z);
        T qxz(q.x * q.z);
        T qxy(q.x * q.y);
        T qyz(q.y * q.z);
        T qwx(q.w * q.x);
        T qwy(q.w * q.y);
        T qwz(q.w * q.z);

        Result[0][0] = T(1) - T(2) * (qyy + qzz);
        Result[0][1] = T(2) * (qxy + qwz);
        Result[0][2] = T(2) * (qxz - qwy);

        Result[1][0] = T(2) * (qxy - qwz);
        Result[1][1] = T(1) - T(2) * (qxx + qzz);
        Result[1][2] = T(2) * (qyz + qwx);

        Result[2][0] = T(2) * (qxz + qwy);
        Result[2][1] = T(2) * (qyz - qwx);
        Result[2][2] = T(1) - T(2) * (qxx + qyy);
        return Result;
    }
    template<typename T>
    Matrix4<T> Scale(const Vector3<T>& v)
    {
        Matrix4<T> m;
        m[0][0] = v.x;
        m[1][1] = v.y;
        m[2][2] = v.z;
        m[3][3] = T(1);
        return m;
    }

    template<typename T>
    inline std::string to_string(const Quaternion<T>& v)
    {
        std::string s;
        s.reserve(64);
        s.append("{x: "); s.append(std::to_string(v.x)); s.append(", ");
        s.append("y: "); s.append(std::to_string(v.y)); s.append(", ");
        s.append("z: "); s.append(std::to_string(v.z)); s.append(", ");
        s.append("w: "); s.append(std::to_string(v.w)); s.append("}");
        return s;
    }
    using Quat4f = Quaternion<float>;
    using Quat4d = Quaternion<double>;

    template<typename T>
    Quaternion<T> Inverse(const Quaternion<T>& q)
    {
        return Quaternion<T>(-q.x, -q.y, -q.z, q.w);
    }

    template<typename T>
    struct Transform2D
    {
        using value_type = T;

        Vector2<T> Position;
        T Rotation;
        Vector2<T> Scale;
    };

    using Transform2Df = Transform2D<float>;
    using Transform2Dd = Transform2D<double>;

    template<typename T>
    struct Transform3D
    {
        using value_type = T;

        Vector3<T> Position;
        Quaternion<T> Rotation;
        Vector3<T> Scale;
    };

    using Transform3Df = Transform3D<float>;
    using Transform3Dd = Transform3D<double>;

    template<typename T>
    struct Color4
    {
        using value_type = T;
        T r, g, b, a;

        const T* get_value_ptr() const { return &r; }

        Color4() : r(0), g(0), b(0), a(0) {}
        Color4(T _r, T _g, T _b) : r(_r), g(_g), b(_b), a(1) {}
        Color4(T _r, T _g, T _b, T _a) : r(_r), g(_g), b(_b), a(_a) {}

        Color4& operator+=(const Color4& c) { r += c.r; g += c.g; b += c.b; a += c.a;  return *this; }
        Color4& operator-=(const Color4& c) { r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }
        Color4& operator*=(const Color4& c) { r *= c.r; g *= c.g; b *= c.b; a *= c.a; return *this; }
        Color4& operator/=(const Color4& c) { r /= c.r; g /= c.g; b /= c.b; a /= c.a; return *this; }
        Color4& operator+=(T s) { r += s; g += s; b += s; a += s;  return *this; }
        Color4& operator-=(T s) { r -= s; g -= s; b -= s; a -= s; return *this; }
        Color4& operator*=(T s) { r *= s; g *= s; b *= s; a *= s; return *this; }
        Color4& operator/=(T s) { r /= s; g /= s; b /= s; a /= s; return *this; }
        bool operator==(const Color4& c) const { return r == c.r && g == c.g && b == c.b && a == c.a; }
    };
    template<typename T> inline Color4<T> operator+(const Color4<T>& a, const Color4<T>& b) { return { a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a }; }
    template<typename T> inline Color4<T> operator-(const Color4<T>& a, const Color4<T>& b) { return { a.r - b.r, a.g - b.g, a.b - b.b, a.a - b.a }; }
    template<typename T> inline Color4<T> operator*(const Color4<T>& a, const Color4<T>& b) { return { a.r * b.r, a.g * b.g, a.b * b.b, a.a * b.a }; }
    template<typename T> inline Color4<T> operator/(const Color4<T>& a, const Color4<T>& b) { return { a.r / b.r, a.g / b.g, a.b / b.b, a.a / b.a }; }
    template<typename T> inline Color4<T> operator+(const Color4<T>& a, T b) { return { a.r + b, a.g + b, a.b + b, a.a + b }; }
    template<typename T> inline Color4<T> operator-(const Color4<T>& a, T b) { return { a.r - b, a.g - b, a.b - b, a.a - b }; }
    template<typename T> inline Color4<T> operator*(const Color4<T>& a, T b) { return { a.r * b, a.g * b, a.b * b, a.a * b }; }
    template<typename T> inline Color4<T> operator/(const Color4<T>& a, T b) { return { a.r / b, a.g / b, a.b / b, a.a / b }; }
    template<typename T> inline Color4<T> operator+(T a, const Color4<T>& b) { return { a + b.r, a + b.g, a + b.b, a + b.a }; }
    template<typename T> inline Color4<T> operator-(T a, const Color4<T>& b) { return { a - b.r, a - b.g, a - b.b, a - b.a }; }
    template<typename T> inline Color4<T> operator*(T a, const Color4<T>& b) { return { a * b.r, a * b.g, a * b.b, a * b.a }; }
    template<typename T> inline Color4<T> operator/(T a, const Color4<T>& b) { return { a / b.r, a / b.g, a / b.b, a / b.a }; }

    template<typename T>
    inline std::string to_string(const Color4<T>& c)
    {
        std::string s;
        s.reserve(64);
        s.append("{r: "); s.append(std::to_string(c.r)); s.append(", ");
        s.append("g: "); s.append(std::to_string(c.g)); s.append(", ");
        s.append("b: "); s.append(std::to_string(c.b)); s.append(", ");
        s.append("a: "); s.append(std::to_string(c.a)); s.append("}");
        return s;
    }

    using Color4b = Color4<uint8_t>;
    using Color4f = Color4<float>;
    using Color4d = Color4<double>;



    template <typename T>
    struct Box3
    {
        Vector3<T> Min;
        Vector3<T> Max;

        Vector3<T> GetCenter() const
        {
            return (Min + Max) * T(0.5);
        }

        Vector3<T> GetExtend() const
        {
            return (Max - Min) * T(0.5);
        }

        Vector3<T> GetSize() const
        {
            return Max - Min;
        }
    };

    using Box3f = Box3<float>;
    using Box3d = Box3<double>;

    template <typename T>
    struct Bounds3
    {
        Vector3<T> Origin{};
        Vector3<T> Extent{};
        T          Sphere{};

        Bounds3() = default;

        Bounds3(const Vector3<T>& origin, const Vector3<T>& extent, T sphere) :
            Origin(origin), Extent(extent), Sphere(sphere)
        {
        }

        explicit Bounds3(const Box3<T>& box) :
            Origin(box.GetCenter()), Extent(box.GetExtend()), Sphere(box.GetSize().Magnitude() * 0.5f)
        {
        }

        Box3<T> GetBox() const
        {
            return {Origin - Extent, Origin + Extent};
        }
    };

    using Bounds3f = Bounds3<float>;
    using Bounds3d = Bounds3<float>;

    template<typename B, typename L>
    Color4<B> FloatColorToBitColor(const Color4<L>& l)
    {
        static_assert(std::is_floating_point_v<L>, "L not integral type");
        static_assert(std::is_integral_v<B>, "B not integral type");

        constexpr int scalar = std::numeric_limits<typename Color4<L>::value_type>::max();
        return { B(l.r * scalar), B(l.g * scalar), B(l.b * scalar), B(l.a * scalar) };
    }

    template<typename L, typename B>
    Color4<L> BitColorToFloatColor(const Color4<B>& b)
    {
        static_assert(std::is_floating_point_v<L>, "L not integral type");
        static_assert(std::is_integral_v<B>, "B not integral type");

        constexpr int scalar = std::numeric_limits<typename Color4<B>::value_type>::max();
        return { L(b.r) / scalar, L(b.g) / scalar, L(b.b) / scalar, L(b.a) / scalar };
    }
}
