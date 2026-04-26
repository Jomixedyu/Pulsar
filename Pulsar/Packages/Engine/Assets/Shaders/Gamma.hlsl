#include "Blit.inc.hlsl"
#include "PostProcessing.inc.hlsl"

float _Gamma;

float4 PSMain(float4 position : SV_Position, float2 texcoord) : SV_Target
{
    float4 color = PP_InColor.Sample(Sampler_PP_InColor, texcoord);
    
    color.rgb = pow(abs(color.rgb), 1.0 / _Gamma);
    
    return color;
}
