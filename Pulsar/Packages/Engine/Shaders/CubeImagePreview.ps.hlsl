#include "PostProcessing.inc.hlsl"

#define FLAGS_GAMMA 0x01
#define FLAGS_EnableCheckerBackground 0x02
#define FLAGS_CHANNEL_R 0x04
#define FLAGS_CHANNEL_G 0x08
#define FLAGS_CHANNEL_B 0x10
#define FLAGS_CHANNEL_A 0x20
#define FLAGS_NORMALMAP 0x40


cbuffer Properties : register(b0, USER_DESCSET)
{
    float4 _CheckerColorA;
    float4 _CheckerColorB;
    float  _GridSize;
    int    _Flags;
}

TextureCube _Image : register(t1, USER_DESCSET);
SamplerState _ImageSampler : register(s1, USER_DESCSET);

float4 main(PPVSOutput v2f) : SV_TARGET
{
    float4 output = float4(0,0,0,1);


    return output;
}