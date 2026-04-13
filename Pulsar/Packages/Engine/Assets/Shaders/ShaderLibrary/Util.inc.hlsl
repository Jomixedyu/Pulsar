#ifndef _SHADERLIBRARY_UTILLIBRARY_HLSL_
#define _SHADERLIBRARY_UTILLIBRARY_HLSL_


namespace util
{
    half SphereMaskSmooth(half3 a, half3 b, half radius, half smooth)
    {
        half dis = length(a - b);
        half mask = smoothstep(radius + smooth, radius - smooth, dis);
        return mask;
    }
    
    half SphereMask(half3 a, half3 b, half radius, half hardness)
    {
        return saturate((1 - distance(a, b) * (1 / max(0.00001000, radius))) * (1 / max(1 - hardness, 0.00001000)));
    }

    float4 Sample1D_Linear(float4 buffer[16], float tNorm)
    {
        float t = (tNorm * 15);
        int base = (int)t;
        float dec = frac(t);
        return lerp(buffer[base], buffer[(base + 1) % 16], dec);
    }
    
    float GetLimitOfFarZ(float poswCS, float offset = 0)
    {
        #if SHL_REVERSED_Z
        return FLT_MIN - offset;
        #else
        return poswCS - 1e-6 + offset * 2;
        #endif
    }
    
    float DepthFade(float4 positionCS, half opacity, half distance)
    {
        float2 uv = positionCS.xy / _ScaledScreenParams.xy;
        float sceneDepth = LinearEyeDepth(SampleSceneDepth(uv), _ZBufferParams);
        float objectDepth = positionCS.w;
        float depthDiff = sceneDepth - objectDepth;
        float alpha = saturate(depthDiff / distance) * opacity;
        return alpha;
    }
    
    void Dither(float4 positionCS, half opacity)
    {
        const float3 magic = float3(0.06711056, 0.00583715, 52.9829189);
        half dither = 1 - saturate(opacity);
        float pattern = frac(magic.z * frac(dot(positionCS.xy + positionCS.z, magic.xy)));
        clip(pattern - dither);
    }

    float3 BezierQuadraticLerp(float3 p0, float3 p1, float3 p2, float t)
    {
        float3 a = lerp(p0, p1, t);
        float3 b = lerp(p1, p2, t);
        return lerp(a, b, t);
    }

    float3 BezierCubicLerp(float3 p0, float3 p1, float3 p2, float3 p3, float t)
    {
        float3 a = lerp(p0, p1, t);
        float3 b = lerp(p1, p2, t);
        float3 c = lerp(p2, p3, t);
        
        float3 d = lerp(a, b, t);
        float3 e = lerp(b, c, t);
        
        return lerp(d, e, t);
    }

    float2 BumpOffset(float2 uv, float height, float3 viewDirTS)
    {
        return viewDirTS.xy * height + uv;
    }
}


#endif

