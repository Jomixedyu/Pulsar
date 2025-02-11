#include "Common.inc.hlsl"
#include "SurfacePS.inc.hlsl"


Texture2D _BaseColorMap : register(t0, USER_DESCSET);
Texture2D _NormalMap    : register(t1, USER_DESCSET);
Texture2D _MRAMap       : register(t2, USER_DESCSET);

SamplerState _BaseColorMapSampler  : register(s0, USER_DESCSET);
SamplerState _NormamlMapSampler    : register(s1, USER_DESCSET);
SamplerState _MRAMapSampler        : register(s2, USER_DESCSET);


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
