#include "Blit.inc.hlsl"
#include "PostProcessing.inc.hlsl"
#include "ShaderLibrary/ColorSpace.inc.hlsl"

float4 PSMain(float4 position : SV_Position, float2 texcoord) : SV_Target
{
    float4 color = PP_InColor.Sample(Sampler_PP_InColor, texcoord);
    
    color.rgb = LinearToSRGB(saturate(color.rgb));
    
    return color;
}
