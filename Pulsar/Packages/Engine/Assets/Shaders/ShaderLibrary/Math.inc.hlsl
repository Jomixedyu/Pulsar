#ifndef _SHADERLIBRARY_MATH_HLSL_
#define _SHADERLIBRARY_MATH_HLSL_

#ifndef PI
#define PI          3.1415926535897932384626
#endif

#ifndef INV_PI
#define INV_PI      0.31830988618379067154
#endif

#ifndef TWO_PI
#define TWO_PI      6.28318530717958647693
#endif

#ifndef INV_2PI
#define INV_2PI     0.15915494309
#endif

#ifndef HALF_PI
#define HALF_PI     1.57079632679489661923
#endif

// #define MATH_LEVEL0
#define MATH_LEVEL1
// #define MATH_LEVEL2

namespace math
{
    float2 SinCos_Level0(float x)
    {
        float2 result;
        sincos(x, result.x, result.y);
        return result;
    }
    float2 SinCos_Level1(float x)
    {
        float2 xx = x * INV_2PI + float2(0, 0.25); // mad
        float2 a = 4.0 * frac(xx) - 2.0; // fract + mad
        return a * (abs(a) - 2.0); // abs + add + mul
    }
    float2 SinCos(float x)
    {
        #if defined(MATH_LEVEL0)
        return SinCos_Level0(x);
        #elif defined(MATH_LEVEL1)
        return SinCos_Level1(x);
        #elif defined(MATH_LEVEL2)
        return SinCos_Level1(x);
        #endif
    }

    float3 Sin_Level0(float3 x)
    {
        return sin(x);
    }
    float3 Sin_Level1(float3 x)
    {
        float3 a = 4.0 * frac(x * INV_2PI) - 2.0;
        return a * (abs(a) - 2.0);
    }
    float3 Sin(float3 x)
    {
        #if defined(MATH_LEVEL0)
        return Sin_Level0(x);
        #elif defined(MATH_LEVEL1)
        return Sin_Level1(x);
        #elif defined(MATH_LEVEL2)
        return Sin_Level1(x);
        #endif
    }

    float3 Cos_Level0(float3 x)
    {
        return cos(x);
    }
    float3 Cos_Level1(float3 x)
    {
        float3 a = 4.0 * frac(x * INV_2PI + 0.25) - 2.0;
        return a * (abs(a) - 2.0);
    }
    float3 Cos(float3 x)
    {
        #if defined(MATH_LEVEL0)
        return Cos_Level0(x);
        #elif defined(MATH_LEVEL1)
        return Cos_Level1(x);
        #elif defined(MATH_LEVEL2)
        return Cos_Level1(x);
        #endif
    }

    
    float FastATanPos(float x)
    {
        float t0 = (x < 1.0) ? x : 1.0 / x;
        float t1 = t0 * t0;
        float poly = 0.0872929;
        poly = -0.301895 + poly * t1;
        poly = 1.0 + poly * t1;
        poly = poly * t0;
        return (x < 1.0) ? poly : HALF_PI - poly;
    }
    float ATan_Level0(float x)
    {
        return atan(x);
    }
    float ATan_Level1(float x)
    {
        float t0 = FastATanPos(abs(x));
        return (x < 0.0) ? -t0 : t0;
    }
    float ATan(float x)
    {
        #if defined(MATH_LEVEL0)
        return ATan_Level0(x);
        #elif defined(MATH_LEVEL1)
        return ATan_Level1(x);
        #endif
    }

    float ATan2_Level0(float y, float x)
    {
        return atan2(y, x);
    }
    float ATan2_Level1(float y, float x)
    {
        return ATan_Level1(y / x) + (y >= 0.0 ? PI : -PI) * (x < 0.0);
    }
    float ATan2_Level2(float y, float x)
    {
        return ATan2_Level1(y, x);
    }
    float ATan2(float y, float x)
    {
        #if defined(MATH_LEVEL0)
        return ATan2_Level0(y, x);
        #elif defined(MATH_LEVEL1)
        return ATan2_Level1(y, x);
        #elif defined(MATH_LEVEL2)
        return ATan2_Level2(y, x);
        #endif
    }
    
    float ACos_Level0(float x)
    {
        return acos(x);
    }
    float ACos_Level1(float inX)
    {
        float x = abs(inX);
        float res = (0.0468878 * x + -0.203471) * x + 1.570796; // p(x)
        res *= sqrt(1.0 - x);
        return (inX >= 0) ? res : PI - res; // Undo range reduction
    }
    float ACos_Level2(float inX)
    {
        return ACos_Level1(inX);
    }
    float ACos(float x)
    {
        #if defined(MATH_LEVEL0)
        return ACos_Level0(x);
        #elif defined(MATH_LEVEL1)
        return ACos_Level1(x);
        #elif defined(MATH_LEVEL2)
        return ACos_Level1(x);
        #endif
    }


    float ASin_Level0(float x)
    {
        return asin(x);
    }
    float ASin_Level1(float x)
    {
        return HALF_PI - ACos_Level1(x);
    }
    float ASin_Level2(float x)
    {
        return ASin_Level1(x);
    }
    float ASin(float x)
    {
        #if defined(MATH_LEVEL0)
        return ASin_Level0(x);
        #elif defined(MATH_LEVEL1)
        return ASin_Level1(x);
        #elif defined(MATH_LEVEL2)
        return ASin_Level2(x);
        #endif
    }
}

#endif