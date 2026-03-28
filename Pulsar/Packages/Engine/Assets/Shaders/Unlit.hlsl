#include "Common.inc.hlsl"
#include "MeshRenderer.inc.hlsl"

float4 _TintColor;

Texture2D _BaseColorMap : register(t1);
SamplerState Sampler__BaseColorMap : register(s1);



StandardVaryings VSMain(StandardAttributes a)
{
    StandardVaryings v = (StandardVaryings)0;
    v.Position = TransformObjectToClip(a.Position);
    v.TexCoord0 = a.TexCoord0;
    return v;
}

float4 PSMain(StandardVaryings v) : SV_Target
{
    float4 tex = _BaseColorMap.Sample(Sampler__BaseColorMap, v.TexCoord0);
    float3 final = tex.rgb * _TintColor.rgb;
    return float4(final, 1);
}

