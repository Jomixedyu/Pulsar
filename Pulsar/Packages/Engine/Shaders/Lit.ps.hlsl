#include "Common.inc.hlsl"
#include "SurfacePS.inc.hlsl"


Texture2D _BaseColorTex : register(t0, space3);
Texture2D _MRATex       : register(t1, space3);

SamplerState _BaseColorSampler : register(s0, space3);
SamplerState _MRASampler       : register(s1, space3);


MaterialAttributes SurfaceMain(InPixelAssembly surf)
{
    MaterialAttributes attr = (MaterialAttributes)0;
    attr.BaseColor = _BaseColorTex.Sample(_BaseColorSampler, surf.TexCoord0);
    float4 MRA = _MRATex.Sample(_MRASampler, surf.TexCoord0);
    attr.Metallic = MRA.r;
    attr.Roughness = MRA.g;
    attr.AmbientOcclusion = MRA.b;
    
    attr.EmissiveColor = _BaseColorTex.Sample(_BaseColorSampler, surf.TexCoord0);

    attr.ShadingModel = SHADING_MODEL_UNLIT;

    return attr;
}
