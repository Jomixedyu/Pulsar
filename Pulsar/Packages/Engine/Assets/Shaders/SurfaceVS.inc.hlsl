#ifndef _ENGINE_SURFACEVS_INC
#define _ENGINE_SURFACEVS_INC

#include "Common.inc.hlsl"
#include "MeshRenderer.inc.hlsl"

void SurfaceVertexMain(inout StandardVaryings v2f);


StandardVaryings main(StandardAttributes a2v)
{
    StandardVaryings v2f = (StandardVaryings) 0;

#ifdef RENDERER_SKINNEDMESH
    float3 skinnedPosition = SkinPosition(a2v.Position,    a2v.BlendIndices, a2v.BlendWeights);
    float3 skinnedNormal   = SkinNormal  (a2v.Normal,      a2v.BlendIndices, a2v.BlendWeights);
    float3 skinnedTangent  = SkinNormal  (a2v.Tangent.xyz, a2v.BlendIndices, a2v.BlendWeights);
    v2f.WorldNormal   = TransformObjectNormalToWorld(skinnedNormal);
    v2f.WorldTangent  = float4(TransformObjectNormalToWorld(skinnedTangent), a2v.Tangent.w);
    v2f.WorldPosition = TransformObjectToWorld(skinnedPosition);
#else
    v2f.WorldNormal   = TransformObjectNormalToWorld(a2v.Normal);
    v2f.WorldTangent  = float4(TransformObjectNormalToWorld(a2v.Tangent.xyz), a2v.Tangent.w);
    v2f.WorldPosition = TransformObjectToWorld(a2v.Position);
#endif

    v2f.TexCoord0 = a2v.TexCoord0;
    v2f.TexCoord1 = a2v.TexCoord1;
    v2f.TexCoord2 = a2v.TexCoord2;
    v2f.TexCoord3 = a2v.TexCoord3;
    v2f.Color = a2v.Color;

    SurfaceVertexMain(v2f);
    v2f.Position = TransformWorldToClip(v2f.WorldPosition.xyz);
    return v2f;
}



#endif