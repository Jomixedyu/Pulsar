#include "Common.inc.hlsl"

OutPixelAssembly main(InPixelAssembly v2f)
{
    OutPixelAssembly p2o;

    float3 l = -WorldBuffer.WorldSpaceLightVector.xyz;

    float3 skylightColor = WorldBuffer.SkyLightColor.xyz * WorldBuffer.SkyLightColor.w;

    float3 dirLightColor = WorldBuffer.WorldSpaceLightColor.xyz * WorldBuffer.WorldSpaceLightColor.w;

    float3 color = dot(v2f.WorldNormal, l) * dirLightColor + skylightColor;
    p2o.Color = float4(color, 1);
    return p2o;
}