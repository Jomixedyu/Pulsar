#include "Common.inc.hlsl"
#include "SurfacePS.inc.hlsl"


Texture2D _BaseColorMap : register(t0, USER_DESCSET);

SamplerState _BaseColorMapSampler  : register(s0, USER_DESCSET);



MaterialAttributes SurfacePixelMain(InPixelAssembly surf)
{
    MaterialAttributes attr = (MaterialAttributes)0;

    attr.EmissiveColor = _BaseColorMap.Sample(_BaseColorMapSampler, surf.TexCoord0).xyz;

    attr.ShadingModel = SHADING_MODEL_UNLIT;

    return attr;
}
