#include "Common.inc.hlsl"

#include "DefaultVSImpl.inc.hlsl"


OutPixelAssembly PSMain(StandardVaryings v2f)
{
    OutPixelAssembly p2o;
    p2o.Color = v2f.Color;
    
    return p2o;
}