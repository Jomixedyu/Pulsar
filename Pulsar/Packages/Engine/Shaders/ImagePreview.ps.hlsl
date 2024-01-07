#include "PostProcessing.inc.hlsl"

#define FLAGS_SRGB 1
#define FLAGS_EnableCheckerBackground 2
#define FLAGS_CHANNEL_R 4
#define FLAGS_CHANNEL_G 8
#define FLAGS_CHANNEL_B 16
#define FLAGS_CHANNEL_A 32


cbuffer Properties : register(b0, space3)
{
    float4 _CheckerColorA;
    float4 _CheckerColorB;
    float  _GridSize;
    int    _Flags;
}

Texture2D _Image : register(t1, space3);
SamplerState _ImageSampler : register(s1, space3);

float4 main(PPVSOutput v2f) : SV_TARGET
{
    float4 output= float4(0,0,0,1);
    float4 imgColor = _Image.Sample(ColorSampler, v2f.TexCoord0);

    if (_Flags & FLAGS_CHANNEL_R)
    {
        output.x = imgColor.x;
    }
    if (_Flags & FLAGS_CHANNEL_G)
    {
        output.y = imgColor.y;
    }
    if (_Flags & FLAGS_CHANNEL_B)
    {
        output.z = imgColor.z;
    }
    if (_Flags & FLAGS_CHANNEL_A)
    {
        output.w = imgColor.w;
    }

    if (_Flags & FLAGS_EnableCheckerBackground)
    {
        float2 channels = saturate(fmod(floor(v2f.Position.xy / _GridSize), 2));
        float crossArea = 1-saturate(channels.x + channels.y);
        float blackArea = channels.x * channels.y;
        float4 checker = lerp(_CheckerColorA, _CheckerColorB, crossArea + blackArea);
        output = lerp(checker, output, output.a);
    }

    if (_Flags & FLAGS_SRGB)
    {
        output = float4(pow(output.xyz, 1/2.2), 1);
    }

    return output;
}