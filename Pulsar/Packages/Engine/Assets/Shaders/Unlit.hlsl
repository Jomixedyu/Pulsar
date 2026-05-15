#include "Common.inc.hlsl"
#include "MeshRenderer.inc.hlsl"

float4 _TintColor;
float4 _EmissiveColor;

Texture2D _BaseColorMap;
SamplerState Sampler__BaseColorMap;



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
    float4 final = tex.rgba * _TintColor.rgba;
    final.rgb += _EmissiveColor.rgb * tex.rgb;
    return final;
}

