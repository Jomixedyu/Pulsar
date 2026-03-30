
#include "Common.inc.hlsl"


#ifdef RENDERER_STATICMESH
#include "MeshRenderer.inc.hlsl"

StandardVaryings VSMain(StandardAttributes a2v)
{
    StandardVaryings v2f = (StandardVaryings) 0;
    v2f.WorldNormal = TransformObjectNormalToWorld(a2v.Normal);
    v2f.TexCoord0 = a2v.TexCoord0;
    v2f.TexCoord1 = a2v.TexCoord1;
    v2f.TexCoord2 = a2v.TexCoord2;
    v2f.TexCoord3 = a2v.TexCoord3;
    v2f.Color = a2v.Color;
    v2f.Position = TransformObjectToClip(a2v.Position);
    return v2f;
}


OutPixelAssembly PSMain(StandardVaryings v2f)
{
    OutPixelAssembly p2o;
    p2o.Color = float4(1, 0, 1, 1);
    
    return p2o;
}

#endif


#ifdef RENDERER_IMAGEPROCESS

#include "PostProcessing.inc.hlsl"

float4 BlitPSMain(Varyings v) : SV_Target
{
    return float4(1, 0, 1, 1);
}


#endif