#include "Common.inc.hlsl"
#include "SurfacePS.inc.hlsl"


Texture2D _BaseColorMap : register(t0, space3);

SamplerState _BaseColorMapSampler  : register(s0, space3);



MaterialAttributes SurfacePixelMain(InPixelAssembly surf)
{
    MaterialAttributes attr = (MaterialAttributes)0;

    attr.EmissiveColor = _BaseColorMap.Sample(_BaseColorMapSampler, surf.TexCoord0);

    attr.ShadingModel = SHADING_MODEL_UNLIT;

    return attr;
}
