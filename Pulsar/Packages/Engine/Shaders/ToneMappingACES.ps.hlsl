#include "PostProcessing.inc.hlsl"
#include "Tonemapping.inc.hlsl"

float4 main(in PPVSOutput v2f) : SV_TARGET
{
	float4 color = PP_InColor.Sample(ColorSampler, v2f.TexCoord0);
	return float4(ToneMapping_ACESFilm(color.xyz), 1);
}

