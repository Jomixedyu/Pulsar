#include "Common.inc.hlsl"

OutPixelAssembly main(InPixelAssembly v2f)
{
    OutPixelAssembly p2o;
    float3 v = normalize(TargetBuffer.CamPosition.xyz - v2f.Position.xyz);
    float3 l = -WorldBuffer.WorldSpaceLightVector.xyz;
    p2o.Color = dot(v, l) + WorldBuffer.SkyLightColor.xyz * WorldBuffer.SkyLightColor.w;
    return p2o;
}