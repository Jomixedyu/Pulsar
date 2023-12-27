#ifndef _ENGINE_DEFAULTVS_INC
#define _ENGINE_DEFAULTVS_INC

#include "Common.inc.hlsl"


InPixelAssembly main(InVertexAssembly a2v)
{
    InPixelAssembly v2f = (InPixelAssembly) 0;
    v2f.WorldNormal = ObjectNormalToWorld(a2v.Normal);
    v2f.TexCoord0 = a2v.TexCoord0;
    v2f.TexCoord1 = a2v.TexCoord1;
    v2f.TexCoord2 = a2v.TexCoord2;
    v2f.TexCoord3 = a2v.TexCoord3;
    v2f.Color = a2v.Color;
    v2f.Position = ObjectToProjection(a2v.Position);
    return v2f;
}


#endif //_ENGINE_DEFAULTVS_INC