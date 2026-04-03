#include "Common.inc.hlsl"
#include "MeshRenderer.inc.hlsl"


Texture2D _BaseColorMap;
SamplerState Sampler__BaseColorMap;



StandardVaryings VSMain(StandardAttributes a)
{
    StandardVaryings v = (StandardVaryings)0;
    v.Position = TransformObjectToClip(a.Position);
    v.TexCoord0 = a.TexCoord0;
    v.WorldNormal = TransformObjectNormalToWorld(a.Normal);
    return v;
}

float4 PSMain(StandardVaryings v) : SV_Target
{
    float NoL = dot(v.WorldNormal, WorldBuffer.WorldSpaceLightVector.xyz);

    float4 tex = _BaseColorMap.Sample(Sampler__BaseColorMap, v.TexCoord0);
    float3 final = WorldBuffer.WorldSpaceLightColor.rgb * tex.rgb * NoL;

    return float4(final, 1);
}

