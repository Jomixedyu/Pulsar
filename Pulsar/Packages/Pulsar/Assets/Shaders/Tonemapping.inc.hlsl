#ifndef _TONEMAPPING_INC
#define _TONEMAPPING_INC

float3 ToneMapping_ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}

float3 ToneMapping_Reinhard(float3 x)
{
    return x / (x + 1.0);
}

// Gran Turismo simplified tonemap (Haarm-Pieter Duiker)
float3 ToneMapping_GT(float3 x)
{
    const float a = 0.22; // linear section start
    const float b = 0.30; // linear section length
    const float c = 0.10;
    const float d = 0.20;
    const float e = 0.01;
    const float f = 0.30;
    return ((x * (a * x + b * c) + d * e) / (x * (a * x + b) + d * f)) - e / f;
}

// Uncharted 2 tonemap (John Hable)
float3 ToneMapping_Uncharted2Helper(float3 x)
{
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 ToneMapping_Uncharted2(float3 x)
{
    const float W = 11.2;
    float3 whiteScale = 1.0 / ToneMapping_Uncharted2Helper(W);
    return ToneMapping_Uncharted2Helper(x) * whiteScale;
}

// Hable filmic tonemap (presentation: "Filmic Tonemapping Operators", GDC 2010)
float3 ToneMapping_Filmic(float3 x)
{
    float3 X = max(float3(0.0, 0.0, 0.0), x - 0.004);
    float3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
    return pow(result, 2.2); // the curve is designed for sRGB output, compensate
}

#endif
