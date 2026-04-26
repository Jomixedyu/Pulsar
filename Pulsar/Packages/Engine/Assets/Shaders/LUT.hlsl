#include "Blit.inc.hlsl"
#include "PostProcessing.inc.hlsl"

Texture2D    _LUTTex;
SamplerState Sampler__LUTTex;

float _Intensity;
int   _LUTSize;
int   _ColorSpace;

// Unity-style LogC encoding (simplified ARRI curve)
float3 LinearToLogC(float3 linearColor)
{
    const float a = 5.5555558;
    const float d = 0.047996;
    const float b = 0.0734998;
    const float c = 0.386036;

    float3 x = linearColor * a + d;
    float3 logC = c + b * log2(x);
    return saturate(logC);
}

float3 LinearToSRGB(float3 lin)
{
    return lin <= 0.0031308 ? lin * 12.92 : pow(lin, 1.0 / 2.4) * 1.055 - 0.055;
}

void GetLut2dUV(float3 uvw, float size, float depth, out float2 outUV0, out float2 outUV1, out float outFrac)
{
    uvw = saturate(uvw);

    float zPos = uvw.z * (depth - 1.0);
    float zSlice0 = floor(zPos);
    float zSlice1 = min(zSlice0 + 1.0, depth - 1.0);
    outFrac = zPos - zSlice0;

    float pixelSize = 1.0 / size;
    float halfPixel = 0.5 * pixelSize;

    float u = uvw.x * (size - 1.0) / size + halfPixel;
    float v = uvw.y * (size - 1.0) / size + halfPixel;

    float sliceWidth = 1.0 / depth;
    float offset0 = zSlice0 * sliceWidth;
    float offset1 = zSlice1 * sliceWidth;

    float u0 = u * sliceWidth + offset0;
    float u1 = u * sliceWidth + offset1;

    outUV0 = float2(u0, v);
    outUV1 = float2(u1, v);
}

float3 SampleLUT(float3 color)
{
    float size = max((float)_LUTSize, 1.0);
    float depth = size;

    float2 uv0, uv1;
    float frac;
    GetLut2dUV(color, size, depth, uv0, uv1, frac);

    float3 sample0 = _LUTTex.Sample(Sampler__LUTTex, uv0).rgb;
    float3 sample1 = _LUTTex.Sample(Sampler__LUTTex, uv1).rgb;

    return lerp(sample0, sample1, frac);
}

float4 PSMain(float4 position : SV_Position, float2 texcoord) : SV_Target
{
    float4 color = PP_InColor.Sample(Sampler_PP_InColor, texcoord);

    if (_Intensity <= 0.0)
        return color;

    float3 lutInput = color.rgb;

    if (_ColorSpace == 1)       // LogC
    {
        lutInput = LinearToLogC(lutInput);
    }
    else if (_ColorSpace == 2)  // sRGB
    {
        lutInput = LinearToSRGB(lutInput);
    }

    float3 graded = SampleLUT(lutInput);

    // LUT output is assumed to be in the target display space:
    // - Linear LUT -> Linear output
    // - LogC LUT   -> sRGB output (no conversion back)
    // - sRGB LUT   -> sRGB output
    // Gamma correction pass should be disabled when using LogC/sRGB LUTs.

    color.rgb = lerp(color.rgb, graded, _Intensity);
    return color;
}
