#include "Blit.inc.hlsl"
#include "Tonemapping.inc.hlsl"
#include "PostProcessing.inc.hlsl"

int _TonemappingMode;
int _Enabled;

float4 PSMain(float4 position : SV_Position, float2 texcoord) : SV_Target
{
    float4 color = PP_InColor.Sample(Sampler_PP_InColor, texcoord);
    
    if (_Enabled == 0)
        return color;
    
    if (_TonemappingMode == 1) {
        color.rgb = ToneMapping_ACESFilm(color.rgb);
    } else if (_TonemappingMode == 2) {
        color.rgb = ToneMapping_Reinhard(color.rgb);
    } else if (_TonemappingMode == 3) {
        color.rgb = ToneMapping_GT(color.rgb);
    }
    
    return color;
}
