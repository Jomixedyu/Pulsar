#include "Common.inc.hlsl"
#include "SurfacePS.inc.hlsl"

cbuffer PerMat : register(b0, USER_DESCSET)
{
    float _RampAtlasIndex;
};

Texture2D _BaseColorMap : register(t1, USER_DESCSET);
SamplerState _BaseColorMapSampler  : register(s1, USER_DESCSET);

Texture2D _RampAtlas : register(t2, USER_DESCSET);
SamplerState _RampAtlasSampler : register(s2, USER_DESCSET);


MaterialAttributes SurfacePixelMain(StandardVaryings surf)
{
    MaterialAttributes attr = (MaterialAttributes)0;
    
    float NdL = dot(surf.WorldNormal, -WorldBuffer.WorldSpaceLightVector.xyz);
    float2 uv = float2(NdL * 0.5 + 0.5, _RampAtlasIndex);
    
    float4 ramp = _RampAtlas.Sample(_RampAtlasSampler, uv);

    attr.EmissiveColor = ramp.xyz * _BaseColorMap.Sample(_BaseColorMapSampler, surf.TexCoord0).xyz;

    attr.ShadingModel = SHADING_MODEL_UNLIT;

    return attr;
}
