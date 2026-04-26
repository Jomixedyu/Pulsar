#ifndef _ENGINE_SURFACEPS_INC
#define _ENGINE_SURFACEPS_INC

#include "DeferredShading.inc.hlsl"
#include "Common.inc.hlsl"
#include "MaterialAttributes.inc.hlsl"
#include "MeshRenderer.inc.hlsl"
#include "CalcLighting.inc.hlsl"

MaterialAttributes SurfacePixelMain(StandardVaryings s);

#ifdef RENDERING_PATH_DEFERRED
OutPixelDeferred main(StandardVaryings v2f)
#else
float4 main(StandardVaryings v2f) : SV_TARGET0
#endif

{
    v2f.WorldNormal = normalize(v2f.WorldNormal);
    
    //calc user pixel shader
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
    return CalcLighting(attr, v2f);
#endif
}

#endif //_ENGINE_SURFACEPS_INC