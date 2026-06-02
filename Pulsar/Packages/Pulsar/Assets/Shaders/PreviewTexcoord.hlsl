#include "Common.inc.hlsl"
#include "DefaultVSImpl.inc.hlsl"

int _ChannelIndex;

float4 PSMain(StandardVaryings v) : SV_Target
{
    float2 uv = v.TexCoord0;
    if (_ChannelIndex == 1) uv = v.TexCoord1;
    if (_ChannelIndex == 2) uv = v.TexCoord2;
    if (_ChannelIndex == 3) uv = v.TexCoord3;

    return float4(uv, 0.0f, 1.0f);
}
