#ifndef _ENGINE_SURFACEPS_INC
#define _ENGINE_SURFACEPS_INC

#include "DeferredShading.inc.hlsl"
#include "Common.inc.hlsl"

#define SHADING_MODEL_UNLIT      0x01
#define SHADING_MODEL_LIT        0x02

struct MaterialAttributes
{
    float3 BaseColor;
    float  Metallic;
    float  Specular;
    float  Roughness;
    float  Anisotropy;
    float3 EmissiveColor;
    float  Opacity;
    float  OpacityMask;
    float3 Normal;
    float3 Tangent;
    float3 SubsurfaceColor;
    float  AmbientOcclusion;
    uint   ShadingModel;
};

struct PointLight
{
    float4 Position;
    float4 Color; // w intensity
};


MaterialAttributes SurfacePixelMain(InPixelAssembly s);

#include "SMLit.inc.hlsl"

float4 CalcLighting(MaterialAttributes attr)
{
    float4 color = float4(0,1,1,1);
    [branch]
    if (attr.ShadingModel & SHADING_MODEL_UNLIT)
    {
        color = float4(attr.EmissiveColor, 1);
    }
    else if(attr.ShadingModel & SHADING_MODEL_LIT)
    {
        color = float4(attr.EmissiveColor, 1);
    }
    return color;
}

#ifdef RENDERING_PATH_DEFERRED
OutPixelDeferred main(InPixelAssembly v2f)
#else
float4 main(InPixelAssembly v2f) : SV_TARGET0
#endif
{
    MaterialAttributes attr = SurfacePixelMain(v2f);
#ifdef RENDERING_PATH_DEFERRED
    OutPixelDeferred o;
    
    o.GBufferA = float4(attr.BaseColor, 1);
    o.GBufferB = float4(attr.Normal, 1);
    o.GBufferC = float4(attr.Metallic, attr.Roughness, attr.AmbientOcclusion, attr.Specular);
    o.GBufferD = 0;
    o.GBufferE = float4(attr.Tangent, 1);
    return o;
#else
    return CalcLighting(attr);
#endif
}

#endif //_ENGINE_SURFACEPS_INC