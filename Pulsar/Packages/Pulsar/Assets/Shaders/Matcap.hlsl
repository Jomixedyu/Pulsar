#include "Common.inc.hlsl"
#include "MeshRenderer.inc.hlsl"
#include "ShaderLibrary/Mapping.inc.hlsl"

Texture2D _BaseColorMap;
SamplerState Sampler__BaseColorMap;



StandardVaryings VSMain(StandardAttributes a)
{
    StandardVaryings v = (StandardVaryings)0;
    v.Position = TransformObjectToClip(a.Position);
    v.WorldNormal = TransformObjectNormalToWorld(a.Normal);
    v.TexCoord0 = a.TexCoord0;
    return v;
}

float4 PSMain(StandardVaryings v) : SV_Target
{
    float3 N = normalize(v.WorldNormal);
    float2 uv = map_MatCapMapping(TransformWorldToViewDir(N));

    float4 tex = _BaseColorMap.Sample(Sampler__BaseColorMap, uv);
    
    return tex;
}

