#ifndef _ENGINE_CALCLIGHTING_INC
#define _ENGINE_CALCLIGHTING_INC


#include "DeferredShading.inc.hlsl"
#include "Common.inc.hlsl"
#include "MaterialAttributes.inc.hlsl"
#include "MeshRenderer.inc.hlsl"

#define SHADING_MODEL_UNLIT      0x01
#define SHADING_MODEL_LIT        0x02


#include "SMLit.inc.hlsl"

float4 CalcLighting(MaterialAttributes attr, StandardVaryings v2f)
{
    float4 color = float4(0,1,1,1);
    [branch]
    if (attr.ShadingModel & SHADING_MODEL_UNLIT)
    {
        color = float4(attr.EmissiveColor, 1);
    }
    else if(attr.ShadingModel & SHADING_MODEL_LIT)
    {
        color = ShadingModel_Lit(attr, v2f);
    }
    return color;
}


#endif