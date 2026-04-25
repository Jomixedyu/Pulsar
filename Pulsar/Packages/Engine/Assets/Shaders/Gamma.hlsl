#include "Blit.inc.hlsl"
#include "PostProcessing.inc.hlsl"

float _Gamma;
int _Enabled;

float4 PSMain(float4 position : SV_Position, float2 texcoord) : SV_Target
{
    float4 color = PP_InColor.Sample(Sampler_PP_InColor, texcoord);
    
    if (_Enabled == 0)
        return color;
    
    color.rgb = pow(abs(color.rgb), 1.0 / _Gamma);
    
    return color;
}
