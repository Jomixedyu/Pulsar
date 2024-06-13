#include "Common.inc.hlsl"
#include "SurfacePS.inc.hlsl"


Texture2D _BaseColorMap : register(t0, space3);
Texture2D _NormalMap    : register(t1, space3);
Texture2D _MRAMap       : register(t2, space3);

SamplerState _BaseColorMapSampler  : register(s0, space3);
SamplerState _NormamlMapSampler    : register(s1, space3);
SamplerState _MRAMapSampler        : register(s2, space3);


MaterialAttributes SurfacePixelMain(InPixelAssembly surf)
{
    MaterialAttributes attr = (MaterialAttributes)0;
    attr.BaseColor = _BaseColorMap.Sample(_BaseColorMapSampler, surf.TexCoord0);
    float4 MRA = _MRAMap.Sample(_MRAMapSampler, surf.TexCoord0);
    attr.Metallic = MRA.r;
    attr.Roughness = MRA.g;
    attr.AmbientOcclusion = MRA.b;
    
    //attr.EmissiveColor = _BaseColorMap.Sample(_BaseColorMapSampler, surf.TexCoord0);

    attr.ShadingModel = SHADING_MODEL_LIT;

    return attr;
}
