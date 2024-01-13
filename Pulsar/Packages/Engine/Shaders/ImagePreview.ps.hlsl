#include "PostProcessing.inc.hlsl"

#define FLAGS_GAMMA 0x01
#define FLAGS_EnableCheckerBackground 0x02
#define FLAGS_CHANNEL_R 0x04
#define FLAGS_CHANNEL_G 0x08
#define FLAGS_CHANNEL_B 0x10
#define FLAGS_CHANNEL_A 0x20
#define FLAGS_NORMALMAP 0x40


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
    float4 output = float4(0,0,0,1);
    if (_Flags & FLAGS_NORMALMAP) output.b = 1;

    float4 imgColor = _Image.Sample(ColorSampler, v2f.TexCoord0);

    if (_Flags & FLAGS_CHANNEL_R) output.x = imgColor.x;
    if (_Flags & FLAGS_CHANNEL_G) output.y = imgColor.y;
    if ((_Flags & FLAGS_CHANNEL_B) && !(_Flags & FLAGS_NORMALMAP)) output.z = imgColor.z;
    if (_Flags & FLAGS_CHANNEL_A) output.w = imgColor.w;

    if (!(_Flags & FLAGS_CHANNEL_R) && !(_Flags & FLAGS_CHANNEL_G) &&
        !(_Flags & FLAGS_CHANNEL_B) && _Flags & FLAGS_CHANNEL_A)
    {
        output.xyz = imgColor.w;
        output.w = 1;
    }

    if (_Flags & FLAGS_EnableCheckerBackground)
    {
        float2 channels = saturate(fmod(floor(v2f.Position.xy / _GridSize), 2));
        float crossArea = 1-saturate(channels.x + channels.y);
        float blackArea = channels.x * channels.y;
        float4 checker = lerp(_CheckerColorA, _CheckerColorB, crossArea + blackArea);
        output = lerp(checker, output, output.a);
    }
    else
    {
        output = float4(lerp(0, output.xyz, output.w), 1);
    }

    if (_Flags & FLAGS_GAMMA)
    {
        output = float4(pow(output.xyz, 1/2.2), output.w);
    }

    return output;
}