#ifndef _ENGINE_SURFACEVS_INC
#define _ENGINE_SURFACEVS_INC

#include "Common.inc.hlsl"
#include "MeshRenderer.inc.hlsl"

void SurfaceVertexMain(inout InPixelAssembly v2f);


InPixelAssembly main(InVertexAssembly a2v)
{
    InPixelAssembly v2f = (InPixelAssembly) 0;
    v2f.WorldNormal = ObjectNormalToWorld(a2v.Normal);
    v2f.TexCoord0 = a2v.TexCoord0;
    v2f.TexCoord1 = a2v.TexCoord1;
    v2f.TexCoord2 = a2v.TexCoord2;
    v2f.TexCoord3 = a2v.TexCoord3;
    v2f.Color = a2v.Color;
    v2f.WorldPosition = ObjectToWorld(a2v.Position);

    SurfaceVertexMain(v2f);
    v2f.Position = WorldToClip(v2f.WorldPosition.xyz);
    return v2f;
}



#endif