#include "Common.inc.hlsl"


OutPixelAssembly main(InPixelAssembly v2f)
{
    OutPixelAssembly p2o;
    p2o.Color = float4(1, 0, 1, 1);
    
    return p2o;
}