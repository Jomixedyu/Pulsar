#pragma once
#include <array>
#include <vector>

namespace pulsar
{
    class SimplePrimitiveUtils
    {
    public:
        template<typename _Vector>
        static std::array<_Vector, 24> CreateBox()
        {
            constexpr auto x = 1.f;
            constexpr auto y = 1.f;
            constexpr auto z = 1.f;

            auto a = _Vector{ -x,  y, -z };
            auto b = _Vector{ -x,  y,  z };
            auto c = _Vector{  x,  y,  z };
            auto d = _Vector{  x,  y, -z };
            auto e = _Vector{ -x, -y, -z };
            auto f = _Vector{ -x, -y,  z };
            auto g = _Vector{  x, -y,  z };
            auto h = _Vector{  x, -y, -z };

            return {
                a,b, b,c, c,d, d,a,
                e,f, f,g, g,h, h,e,
                a,e, b,f, c,g, d,h
            };
        }

        template<typename _Vector>
        static std::vector<_Vector> CreateSphere(int segmentCount)
        {
            constexpr float deg2rad = 0.017453292519943f;

            constexpr float radius = 1.0f;
            std::vector<_Vector> points;
            auto theta = deg2rad * (360.f / (float)segmentCount);
            for (int i = 0; i < segmentCount; ++i)
            {
                auto& p1 = points.emplace_back();
                p1 = _Vector{cos(theta * i), 0, sin(theta * i)} * radius;

                auto& p2 = points.emplace_back();
                p2 = _Vector{cos(theta * float(i + 1)), 0, sin(theta * float(i + 1))} * radius;
            }
            for (int i = 0; i < segmentCount; ++i)
            {
                auto& p1 = points.emplace_back();
                p1 = _Vector{0, cos(theta * i), sin(theta * i)} * radius;

                auto& p2 = points.emplace_back();
                p2 = _Vector{0, cos(theta * float(i + 1)), sin(theta * float(i + 1))} * radius;
            }
            for (int i = 0; i < segmentCount; ++i)
            {
                auto& p1 = points.emplace_back();
                p1 = _Vector{cos(theta * i), sin(theta * i), 0} * radius;

                auto& p2 = points.emplace_back();
                p2 = _Vector{cos(theta * float(i + 1)), sin(theta * float(i + 1)), 0} * radius;
            }
            return points;
        }
    };
} // namespace pulsar