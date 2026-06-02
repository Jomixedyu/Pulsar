#ifndef _COLORSPACE_INC
#define _COLORSPACE_INC

// sRGB EOTF (IEC 61966-2-1): linear -> sRGB
float3 LinearToSRGB(float3 lin)
{
    return lin <= 0.0031308 ? lin * 12.92 : pow(lin, 1.0 / 2.4) * 1.055 - 0.055;
}

// sRGB OETF (IEC 61966-2-1): sRGB -> linear
float3 SRGBToLinear(float3 srgb)
{
    return srgb <= 0.04045 ? srgb / 12.92 : pow((srgb + 0.055) / 1.055, 2.4);
}

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

#endif
