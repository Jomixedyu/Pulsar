#include "Common.inc.hlsl"
#include "SurfacePS.inc.hlsl"



MaterialAttributes SurfacePixelMain(StandardVaryings surf)
{
    MaterialAttributes attr = (MaterialAttributes)0;

    half3 weight = pow(abs(surf.WorldNormal), 1);
    half3 uvWeight = weight /(weight.x+weight.y+weight.z);
    
    float3 d = frac(surf.WorldPosition.xyz);
    
    float3 e = step(0.02, d) * step(d, 0.98);

    float z = e.x * e.y * uvWeight.z;
    float y = e.x * e.z * uvWeight.y;
    float x = e.z * e.y * uvWeight.x;

    float c = saturate(x + y + z);
    
    attr.BaseColor = lerp(float3(0.5), float3(1), c);
    attr.Metallic =  lerp(float3(0.6), float3(0.1), c);
    attr.Roughness = lerp(float3(0.5), float3(1), c);

    attr.ShadingModel = SHADING_MODEL_LIT;

    return attr;
}
