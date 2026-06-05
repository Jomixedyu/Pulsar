#include "Common.inc.hlsl"

#include "DefaultVSImpl.inc.hlsl"



float4 PSMain(StandardVaryings v2f) : SV_Target
{
    return v2f.Color;
}