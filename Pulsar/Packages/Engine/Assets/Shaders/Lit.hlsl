#include "Common.inc.hlsl"
#include "MeshRenderer.inc.hlsl"
#include "CalcLighting.inc.hlsl"


Texture2D _BaseMap;
SamplerState Sampler__BaseMap;

Texture2D _MRAMap;
SamplerState Sampler__MRAMap;


StandardVaryings VSMain(StandardAttributes a)
{
    StandardVaryings v = (StandardVaryings)0;
    v.Position = TransformObjectToClip(a.Position);
    v.TexCoord0 = a.TexCoord0;
    return v;
}

MaterialAttributes PixelAttrs(StandardVaryings v)
{
    MaterialAttributes attr = (MaterialAttributes)0;
    float4 basemap = _BaseMap.Sample(Sampler__BaseMap, v.TexCoord0);
    attr.BaseColor = basemap.rgb;
    attr.Opacity = basemap.a;
    float4 mixmap = _MRAMap.Sample(Sampler__MRAMap, v.TexCoord0);
    attr.Metallic = mixmap.r;
    attr.Roughness = mixmap.g;
    attr.AmbientOcclusion = mixmap.b;
    attr.Normal = v.WorldNormal;
    attr.ShadingModel = SHADING_MODEL_LIT;
    
    return attr;
}

float4 PSMain(StandardVaryings v) : SV_Target
{
    return ShadingModel_Lit(PixelAttrs(v), v);
}

