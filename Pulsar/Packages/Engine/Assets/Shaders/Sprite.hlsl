#include "Common.inc.hlsl"
#include "MeshRenderer.inc.hlsl"

float4 _TintColor;
float4 _EmissiveColor;

Texture2D _BaseColorMap;
SamplerState Sampler__BaseColorMap;

float _Rows;
float _Columns;
float _Index;

StandardVaryings VSMain(StandardAttributes a)
{
    StandardVaryings v = (StandardVaryings)0;
    v.Position = TransformObjectToClip(a.Position);
    v.TexCoord0 = a.TexCoord0;
    return v;
}

float4 PSMain(StandardVaryings v) : SV_Target
{
    float2 uv = v.TexCoord0;

    int rows = (int)_Rows;
    int columns = (int)_Columns;
    int index = (int)_Index;

    int r = index / columns;
    int c = index % columns;

    uv.x = (c + v.TexCoord0.x) / columns;
    uv.y = (r + v.TexCoord0.y) / rows;

    float4 tex = _BaseColorMap.Sample(Sampler__BaseColorMap, uv);
    float4 final = tex.rgba * _TintColor.rgba;
    final.rgb += _EmissiveColor.rgb * tex.rgb;
    return final;
}

