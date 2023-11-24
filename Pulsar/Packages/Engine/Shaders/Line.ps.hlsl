#include "Common.inc.hlsl"

OutPixelAssembly main(InPixelAssembly v2f)
{
    OutPixelAssembly p2o;
    p2o.Color = v2f.Color;
    return p2o;
}