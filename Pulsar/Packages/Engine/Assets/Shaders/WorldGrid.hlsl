#include "Common.inc.hlsl"
#include "MeshRenderer.inc.hlsl"
#include "SMLit.inc.hlsl"

Texture2D _BaseColorMap : register(t1);
SamplerState Sampler__BaseColorMap : register(s1);


StandardVaryings VSMain(StandardAttributes a)
{
    StandardVaryings v = (StandardVaryings)0;
    v.Position = TransformObjectToClip(a.Position);
    v.WorldPosition = TransformObjectToWorld(a.Position);
    v.TexCoord0 = a.TexCoord0;
    v.WorldNormal = TransformObjectNormalToWorld(a.Normal);
    return v;
}

float4 PSMain(StandardVaryings v) : SV_Target
{
    MaterialAttributes attr = (MaterialAttributes)0;

    half3 weight = pow(abs(v.WorldNormal), 1);
    half3 uvWeight = weight /(weight.x + weight.y + weight.z);
    
    float3 d = frac(v.WorldPosition.xyz);
    
    float3 e = step(0.02, d) * step(d, 0.98);

    float z = e.x * e.y * uvWeight.z;
    float y = e.x * e.z * uvWeight.y;
    float x = e.z * e.y * uvWeight.x;

    float c = saturate(x + y + z);
    
    attr.BaseColor = lerp(0.5, 1, c);
    attr.Metallic =  lerp(0.6, 0.1, c);
    attr.Roughness = lerp(0.5, 1, c);

    return ShadingModel_Lit(attr, v);
}

