#include "Common.inc.hlsl"
#include "SurfacePS.inc.hlsl"


Texture2D _BaseColorMap : register(t0, USER_DESCSET);

SamplerState _BaseColorMapSampler  : register(s0, USER_DESCSET);

const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

MaterialAttributes SurfacePixelMain(InPixelAssembly surf)
{
    MaterialAttributes attr = (MaterialAttributes)0;
    
    float2 uv = SampleSphericalMap(surf.WorldNormal);
    attr.EmissiveColor =  _BaseColorMap.Sample(_BaseColorMapSampler, uv);

    attr.ShadingModel = SHADING_MODEL_UNLIT;

    return attr;
}
