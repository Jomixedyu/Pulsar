#include "Common.inc.hlsl"
#include "MeshRenderer.inc.hlsl"

cbuffer shcbuf : register(b0, USER_DESCSET)
{
    float4 _Tint;
}

Texture2D _DiffuseTex : register(t1, USER_DESCSET);

SamplerState _DiffuseTexSampler : register(s1, USER_DESCSET);

OutPixelAssembly main(InPixelAssembly v2f)
{
    OutPixelAssembly p2o;

    float3 l = -WorldBuffer.WorldSpaceLightVector.xyz;

    float3 skylightColor = WorldBuffer.SkyLightColor.xyz * WorldBuffer.SkyLightColor.w;

    float3 dirLightColor = WorldBuffer.WorldSpaceLightColor.xyz * WorldBuffer.WorldSpaceLightColor.w;

    float3 lightingcolor = saturate(dot(normalize(v2f.WorldNormal), l)) * dirLightColor * _Tint.xyz;
    float3 color = lightingcolor * _DiffuseTex.Sample(_DiffuseTexSampler, v2f.TexCoord0).xyz;

    p2o.Color = float4(color + skylightColor, 1);

    return p2o;
}