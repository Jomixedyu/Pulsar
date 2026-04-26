#include "Common.inc.hlsl"
#include "MeshRenderer.inc.hlsl"


Texture2D _BaseColorMap;
SamplerState Sampler__BaseColorMap;



StandardVaryings VSMain(StandardAttributes a2v)
{
    StandardVaryings v2f = (StandardVaryings)0;

    v2f.TexCoord0 = a2v.TexCoord0;
    
    #if defined(RENDERER_SKINNEDMESH)
        float3 skinnedPosition = SkinPosition(a2v.Position, a2v.BlendIndices, a2v.BlendWeights);
        float3 skinnedNormal   = SkinNormal(a2v.Normal, a2v.BlendIndices, a2v.BlendWeights);
        v2f.Position    = TransformObjectToClip(skinnedPosition);
        v2f.WorldNormal = TransformObjectNormalToWorld(skinnedNormal);
    #else
        v2f.Position    = TransformObjectToClip(a2v.Position);
        v2f.WorldNormal = TransformObjectNormalToWorld(a2v.Normal);
    #endif
    
    return v2f;
}

float4 PSMain(StandardVaryings v) : SV_Target
{
    float NoL = dot(v.WorldNormal, WorldBuffer.WorldSpaceLightVector.xyz);

    float4 tex = _BaseColorMap.Sample(Sampler__BaseColorMap, v.TexCoord0);
    float3 final = WorldBuffer.WorldSpaceLightColor.rgb * tex.rgb * NoL;

    return float4(final, 1);
}

