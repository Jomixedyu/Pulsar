
#include "Common.inc.hlsl"

#if defined(RENDERER_STATICMESH) || defined(RENDERER_SKINNEDMESH)
#include "MeshRenderer.inc.hlsl"

StandardVaryings VSMain(StandardAttributes a2v)
{
    StandardVaryings v2f = (StandardVaryings) 0;

    #if defined(RENDERER_SKINNEDMESH)
        float3 skinnedPosition = SkinPosition(a2v.Position, a2v.BlendIndices, a2v.BlendWeights);
        v2f.Position = TransformObjectToClip(skinnedPosition);
    #else
        v2f.Position = TransformObjectToClip(a2v.Position);
    #endif
    
    return v2f;
}

float4 PSMain(StandardVaryings v2f) : SV_Target
{
    return float4(0, 1, 1, 1);
}
#endif


#ifdef RENDERER_IMAGEPROCESS

#include "PostProcessing.inc.hlsl"

float4 BlitPSMain() : SV_Target
{
    return float4(0, 1, 1, 1);
}


#endif