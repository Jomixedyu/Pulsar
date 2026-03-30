
#include "Common.inc.hlsl"

#ifdef RENDERER_STATICMESH
#include "MeshRenderer.inc.hlsl"

StandardVaryings VSMain(StandardAttributes a2v)
{
    StandardVaryings v2f = (StandardVaryings) 0;
    v2f.Position = TransformObjectToClip(a2v.Position);
    return v2f;
}

float4 PSMain(StandardVaryings v2f) : SV_Target
{
    return float4(0, 1, 1, 1);
}
#endif


#ifdef RENDERER_IMAGEPROCESS

#include "PostProcessing.inc.hlsl"

float4 BlitPSMain(Varyings v) : SV_Target
{
    return float4(0, 1, 1, 1);
}


#endif