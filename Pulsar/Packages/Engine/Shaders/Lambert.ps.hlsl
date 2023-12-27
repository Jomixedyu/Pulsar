#include "Common.inc.hlsl"


cbuffer shcbuf : register(b0, space3)
{
    float4 _Tint;
}

Texture2D _DiffuseTex : register(t1, space3);

SamplerState NormalSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

OutPixelAssembly main(InPixelAssembly v2f)
{
    OutPixelAssembly p2o;

    float3 l = -WorldBuffer.WorldSpaceLightVector.xyz;

    float3 skylightColor = WorldBuffer.SkyLightColor.xyz * WorldBuffer.SkyLightColor.w;

    float3 dirLightColor = WorldBuffer.WorldSpaceLightColor.xyz * WorldBuffer.WorldSpaceLightColor.w;

    float3 lightingcolor = saturate(dot(normalize(v2f.WorldNormal), l)) * dirLightColor * _Tint.xyz;
    // float3 color = lightingcolor * _Tint.xyz * _Color.Sample(NormalSampler, v2f.TexCoord0).xyz;
    p2o.Color = float4(lightingcolor + skylightColor, 1);

    return p2o;
}